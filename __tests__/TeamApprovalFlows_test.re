open Jest;
open JestDom;
open ReactTestingLibrary;

[%raw "require('isomorphic-fetch')"];

[@bs.module "msw/node"] [@bs.variadic]
external setupServer: array('requestHandler) => 'worker = "setupServer";

[@bs.module "msw"] external rest: 'rest = "rest";

let teams: array(Types.team) = [|
  {
    id: TeamId("TEAM1"),
    name: "Marketing",
    userIds: [|UserId("USR1"), UserId("USR3")|],
  },
  {
    id: TeamId("TEAM2"),
    name: "Product & Engineering",
    userIds: [|
      UserId("USR2"),
      UserId("USR3"),
      UserId("USR7"),
      UserId("USR8"),
      UserId("USR9"),
    |],
  },
  {
    id: TeamId("TEAM3"),
    name: "Finance",
    userIds: [|UserId("USR4"), UserId("USR5")|],
  },
|];

let server =
  setupServer([|
    rest##get(
      "http://localhost:1234/spx-development/contents/teams", (_req, res, ctx) => {
      res(ctx##json(teams))
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
      |> getByRole(~matcher=`Str("heading"), _)
      |> expect
      |> toBeInTheDocument
    });

    test("shows a list of X elements", () => {
      <TeamApprovalFlows.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("listitem"), _)
      |> Array.length(_)
      |> Expect.expect
      |> Expect.toBe(3)
    });

    test("shows the name of a team", () => {
      <TeamApprovalFlows.TeamList teams />
      |> render(_)
      |> getAllByRole(~matcher=`Str("listitem"), _)
      |> Array.get(_, 0)
      |> expect
      |> toHaveTextContent(`Str("Marketing"), _)
    });

    testPromise("fetches data from api", () => {
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
  })
});
