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
      UserId("USR3"),
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
      <TeamList.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("heading"), _)
      |> Array.get(_, 0)
      |> expect
      |> toHaveTextContent(`Str("Teams approval flows"), _)
    });

    test("shows the name of a team", () => {
      <TeamList.TeamList teams />
      |> render(_)
      |> getAllByTestId(~matcher=`Str("team-list-item"), _)
      |> Array.get(_, 0)
      |> expect
      |> toHaveTextContent(`Str("Marketing"), _)
    });

    testPromise("fetches teams from api", () => {
      let apiOptions: ApiContext.options = {baseUrl: "http://localhost:1234"};
      <ApiContext.Provider value=apiOptions>
        <TeamList />
      </ApiContext.Provider>
      |> render(_)
      |> findAllByTestId(~matcher=`Str("team-list-item"), _)
      |> Js.Promise.then_(elements =>
           elements
           |> Array.get(_, 0)
           |> expect
           |> toHaveTextContent(`Str("Marketing"), _)
           |> Js.Promise.resolve
         );
    });

    testPromise("shows the first 3 members of a team", () => {
      <TeamList.TeamList teams users />
      |> render(_)
      |> findAllByTestId(~matcher=`Str("team-list-item"), _)
      |> Js.Promise.then_(elements =>
           elements
           |> Array.get(_, 0)
           |> expect
           |> toHaveTextContent(
                `Str(
                  "MarketingMembersRalph RomeroTiffany FrazierSandra ReedApprovers",
                ),
                _,
              )
           |> Js.Promise.resolve
         )
    });
  })
});
