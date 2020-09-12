open Jest;
open JestDom;
open ReactTestingLibrary;

describe("As an Admin", () => {
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
  });
});
