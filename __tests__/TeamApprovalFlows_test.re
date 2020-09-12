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

let teams: array(teamWithUsers) = [|
  {id: TeamId("TEAM1"), name: "Marketing", users: [|users[0], users[2]|]},
  {
    id: TeamId("TEAM2"),
    name: "Product & Engineering",
    users: [|users[1], users[3], users[4], users[5]|],
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
afterEach(() => {server##resetHandlers()});
afterAll(() => server##close());

describe("As an Admin", () => {
  describe("shows the list of the teams", () => {
    test("shows the component", () => {
      <TeamApprovalFlows.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("heading"), _)
      |> Array.get(_, 0)
      |> expect
      |> toHaveTextContent(`Str("Teams approval flows"), _)
    });

    test("shows a list of X elements", () => {
      <TeamApprovalFlows.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("listitem"), _)
      |> Array.length(_)
      |> Expect.expect
      |> Expect.toBe(7)
    });

    test("shows the name of a team", () => {
      <TeamApprovalFlows.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("listitem"), _)
      |> Array.get(_, 0)
      |> expect
      |> toHaveTextContent(`Str("Marketing"), _)
    });

    testPromise("fetches teams from api", () => {
      let apiOptions: ApiContext.options = {baseUrl: "http://localhost:1234"};
      <ApiContext.Provider value=apiOptions>
        <TeamApprovalFlows />
      </ApiContext.Provider>
      |> render(_)
      |> findAllByRole(~matcher=`Str("listitem"), _)
      |> Js.Promise.then_(elements =>
           elements
           |> Array.get(_, 0)
           |> expect
           |> toHaveTextContent(`Str("Marketing"), _)
           |> Js.Promise.resolve
         );
    });

    describe("shows the first 3 members of a team", () => {
      test("shows the name of a team", () => {
        <TeamApprovalFlows.TeamList teams />
        |> render(_)
        |> getAllByRole(~matcher=`Str("list"), _)
        |> Array.get(_, 2)
        |> expect
        |> toHaveTextContent(`Str("Ralph RomeroSandra ReedJason Casey"), _)
      })
    });
  })
});
