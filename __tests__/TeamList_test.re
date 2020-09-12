open Jest;
open JestDom;
open ReactTestingLibrary;
open Types;

[%raw "require('isomorphic-fetch')"];

[@bs.module "@testing-library/react"] external screen: 'screen = "screen";

[@bs.module "msw/node"] [@bs.variadic]
external setupServer: array('requestHandler) => 'worker = "setupServer";

[@bs.module "msw"] external rest: 'rest = "rest";

let users: array(user) = [|
  {
    id: UserId("USR1"),
    firstName: "Eugene",
    lastName: "Tran",
    email: "eugene.tran@spendesk.com",
  },
  {
    id: UserId("USR2"),
    firstName: "Ralph",
    lastName: "Romero",
    email: "ralph.romero@spendesk.com",
  },
  {
    id: UserId("USR3"),
    firstName: "Tiffany",
    lastName: "Frazier",
    email: "tiffany.frazier@spendesk.com",
  },
  {
    id: UserId("USR4"),
    firstName: "Sandra",
    lastName: "Reed",
    email: "sandra.reed@spendesk.com",
  },
  {
    id: UserId("USR5"),
    firstName: "Jason",
    lastName: "Casey",
    email: "jason.casey@spendesk.com",
  },
  {
    id: UserId("USR6"),
    firstName: "Stacy",
    lastName: "Smith",
    email: "stacy.smith@spendesk.com",
  },
|];

let teams: array(team) = [|
  {
    id: TeamId("TEAM1"),
    name: "Marketing",
    userIds: [|
      UserId("USR2"),
      UserId("USR4"),
      UserId("USR5"),
      UserId("USR6"),
    |],
  },
  {
    id: TeamId("TEAM2"),
    name: "Product & Engineering",
    userIds: [|UserId("USR1"), UserId("USR3")|],
  },
|];

let approvalFlows: array(approvalFlow) = [|
  {
    teamId: TeamId("TEAM1"),
    thresholds: [|
      {min: 0.0, max: 500.0, userId: UserId("USR2")},
      {min: 500.0, max: 1000.0, userId: UserId("USR4")},
      {min: 1000.0, max: 5000.0, userId: UserId("USR5")},
      {min: 5.000, max: infinity, userId: UserId("USR6")},
    |],
  },
|];

let server =
  setupServer([|
    rest##get(
      "http://localhost:1234/spx-development/contents/teams", (_req, res, ctx) => {
      res(ctx##json(teams))
    }),
    rest##get(
      "http://localhost:1234/spx-development/contents/users", (_req, res, ctx) => {
      res(ctx##json(users))
    }),
  |]);

beforeAll(() => server##listen());
afterEach(() => server##resetHandlers());
afterAll(() => server##close());

describe("As an Admin", () => {
  describe("shows the list of the teams", () => {
    test("shows the component", () => {
      let result = <TeamList.TeamList teams /> |> render(_);

      result
      |> getByTestId(~matcher=`Str("team-list"), _)
      |> expect
      |> toBeInTheDocument;
    });

    test("shows the name of a team", () => {
      let result = <TeamList.TeamList teams /> |> render(_);

      result
      |> getByTestId(~matcher=`Str("team-list-item-TEAM1"), _)
      |> expect
      |> toHaveTextContent(`Str("Marketing"), _);
    });

    testPromise("fetches teams from api", () => {
      let apiOptions: ApiContext.options = {baseUrl: "http://localhost:1234"};
      let result =
        <ApiContext.Provider value=apiOptions>
          <TeamList />
        </ApiContext.Provider>
        |> render(_);

      result
      |> findByTestId(~matcher=`Str("team-list-item-TEAM1"), _)
      |> Js.Promise.then_(element =>
           element
           |> expect
           |> toHaveTextContent(`Str("Marketing"), _)
           |> Js.Promise.resolve
         );
    });

    testPromise("fetches users from api", () => {
      let apiOptions: ApiContext.options = {baseUrl: "http://localhost:1234"};
      let result =
        <ApiContext.Provider value=apiOptions>
          <TeamList />
        </ApiContext.Provider>
        |> render(_);

      result
      |> findByTestId(~matcher=`Str("user-list-item-USR2"), _)
      |> Js.Promise.then_(element =>
           element
           |> expect
           |> toHaveTextContent(`Str("Ralph Romero"), _)
           |> Js.Promise.resolve
         );
    });

    test("shows the first 3 members of a team", () => {
      let result = <TeamList.TeamList teams users /> |> render(_);

      result
      |> getByTestId(~matcher=`Str("team-list-item-TEAM1"), _)
      |> expect
      |> toHaveTextContent(
           `RegExp(
             [%re "/^MarketingMembersRalph RomeroSandra ReedJason Casey$/"],
           ),
           _,
         );
    });

    test("shows the first 3 approvers of a team", () => {
      let result =
        <TeamList.TeamList teams users approvalFlows /> |> render(_);

      result
      |> getByTestId(~matcher=`Str("team-list-item-TEAM1"), _)
      |> expect
      |> toHaveTextContent(
           `RegExp(
             [%re
               "/^MarketingMembersRalph RomeroSandra ReedJason CaseyApproversRalph RomeroSandra ReedJason Casey$/"
             ],
           ),
           _,
         );
    });

    testPromise("when click on a team it shows the approval flow", () => {
      let result =
        <TeamList.TeamList teams users approvalFlows /> |> render(_);

      result
      |> getByTestId(~matcher=`Str("team-list-item-TEAM1"), _)
      |> FireEvent.click(_)
      |> ignore;

      result
      |> findByTestId(~matcher=`Str("team-approval-flow"), _)
      |> Js.Promise.then_(element =>
           element |> expect |> toBeInTheDocument |> Js.Promise.resolve
         );
    });
  });

  describe("shows the list of approval flows", () => {
    test("closes the screen", () => {
      let mockFn = JestJs.fn(() => ());
      let fn = MockJs.fn(mockFn);

      let result =
        <TeamApprovalFlow
          teamName="test"
          users
          thresholds={approvalFlows[0].thresholds}
          onChange={_ => ()}
          onClose=fn
        />
        |> render(_);

      result
      |> getAllByRole(~matcher=`Str("button"), _)
      |> Array.get(_, 0)
      |> FireEvent.click(_)
      |> ignore;

      mockFn
      |> MockJs.calls
      |> Expect.expect
      |> Expect.toEqual([|[%raw {|undefined|}]|], _);
    });

    test("creates a new threshold in an approval flow", () => {
      let mockFn = JestJs.fn(_ => ());
      let fn = MockJs.fn(mockFn);

      let result =
        <TeamApprovalFlow
          teamName="test"
          users
          thresholds={approvalFlows[0].thresholds}
          onChange=fn
          onClose={() => ()}
        />
        |> render(_);

      result
      |> getByLabelText(~matcher=`Str("From"), _)
      |> FireEvent.change(~eventInit={
                            "target": {
                              "value": "5000",
                            },
                          })
      |> ignore;

      result
      |> getByLabelText(~matcher=`Str("To"), _)
      |> FireEvent.change(~eventInit={
                            "target": {
                              "value": "5500",
                            },
                          })
      |> ignore;

      result
      |> getByLabelText(~matcher=`Str("User"), _)
      |> FireEvent.change(~eventInit={
                            "target": {
                              "value": "USR4",
                            },
                          })
      |> ignore;

      result
      |> getByText(~matcher=`Str("Confirm"), _)
      |> FireEvent.click(_)
      |> ignore;

      mockFn
      |> MockJs.calls
      |> Belt.Array.get(_, 0)
      |> Belt.Option.getWithDefault(_, [||])
      |> Belt.Array.get(_, 4)
      |> Expect.expect
      |> Expect.toEqual(
           Some({"max": "5500", "min": "5000", "userId": "USR4"}->Obj.magic),
           _,
         );
    });

    testPromise("updates a bound threshold in an approval flow", () => {
      let mockFn = JestJs.fn(_ => ());
      let fn = MockJs.fn(mockFn);

      let result =
        <TeamApprovalFlow
          teamName="test"
          users
          thresholds={approvalFlows[0].thresholds}
          onChange=fn
          onClose={() => ()}
        />
        |> render(_);

      result
      |> getAllByRole(~matcher=`Str("button"), _)
      |> Array.get(_, 1)
      |> FireEvent.click(_)
      |> ignore;

      result
      |> findByLabelText(~matcher=`Str("To"), _)
      |> Js.Promise.then_(element =>
           element
           |> expect
           |> toHaveValue(`Num(500), _)
           |> Js.Promise.resolve
         );
    });

    testPromise("updates a user threshold in an approval flow", () => {
      let mockFn = JestJs.fn(_ => ());
      let fn = MockJs.fn(mockFn);

      let result =
        <TeamApprovalFlow
          teamName="test"
          users
          thresholds={approvalFlows[0].thresholds}
          onChange=fn
          onClose={() => ()}
        />
        |> render(_);

      result
      |> getAllByRole(~matcher=`Str("button"), _)
      |> Array.get(_, 1)
      |> FireEvent.click(_)
      |> ignore;

      result
      |> findByLabelText(~matcher=`Str("User"), _)
      |> Js.Promise.then_(element =>
           element
           |> expect
           |> toHaveValue(`Str("USR2"), _)
           |> Js.Promise.resolve
         );
    });

    testPromise("deletes a threshold in an approval flow", () => {
      let mockFn = JestJs.fn(_ => ());
      let fn = MockJs.fn(mockFn);

      let result =
        <TeamApprovalFlow
          teamName="test"
          users
          thresholds={approvalFlows[0].thresholds}
          onChange=fn
          onClose={() => ()}
        />
        |> render(_);

      result
      |> getAllByRole(~matcher=`Str("button"), _)
      |> Array.get(_, 2)
      |> FireEvent.click(_)
      |> ignore;

      result
      |> findByTestId(~matcher=`Str("threshold-list-item-0"), _)
      |> Js.Promise.then_(element =>
           element
           |> expect
           |> toHaveTextContent(
                `RegExp(
                  [%re
                    "/^From 500 to 1000€ needs approval by Sandra ReedEditDelete$/"
                  ],
                ),
                _,
              )
           |> Js.Promise.resolve
         );
    });
  });
});
