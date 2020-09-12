open Types;

/**
 * Fetcher utils
 */
type fetchResult('data, 'error) = {
  data: Js.Nullable.t('data),
  error: Js.Nullable.t('error),
};

[@bs.module "swr"]
external useSwr: (string, 'fn) => fetchResult('data, 'error) = "default";

let useFetcher = path => {
  let apiOptions = React.useContext(ApiContext.context);

  let fetcher = () =>
    Fetch.fetch(apiOptions.baseUrl ++ path)
    |> Js.Promise.then_(Fetch.Response.json);

  let {data, error} = useSwr(path, fetcher);

  let data = data->Js.Nullable.toOption;
  let error = error->Js.Nullable.toOption;
  let loading =
    switch (data, error) {
    | (None, None) => true
    | _ => false
    };

  (data, loading, error);
};

/**
 * LocalStorage utils
 */
[@bs.val] [@bs.scope "localStorage"]
external getItem: string => string = "getItem";

[@bs.val] [@bs.scope "localStorage"]
external setItem: (string, string) => unit = "setItem";

let saveApprovalFlows = (approvalFlows: array(approvalFlow)) => {
  setItem("approval-flows", approvalFlows->Obj.magic->Js.Json.stringify);
};

let useLocalApprovalFlows = () => {
  let fetcher = () =>
    (
      try(Some(Js.Json.parseExn(getItem("approval-flows"))->Obj.magic)) {
      | _ => None
      }
    )
    ->Js.Nullable.fromOption
    ->Js.Promise.resolve;

  let {data, error} = useSwr("approval-flows", fetcher);

  let data =
    switch (data->Js.Nullable.toOption) {
    | Some(_) as data => data
    | None => Some([||])
    };
  let error = error->Js.Nullable.toOption;
  let loading = false;

  (data, loading, error);
};

/**
 * Data conversion utils
 */
let getUserByUserId = (users: array(user), id: userId) =>
  users->Belt.Array.getBy(user => user.id === id);

let getUsersByUserIds = (users: array(user), ids: array(userId)) =>
  ids->Belt.Array.keepMap(id => getUserByUserId(users, id));

let getThresholdsByTeamId =
    (approvalFlows: array(approvalFlow), id: teamId): array(threshold) => {
  approvalFlows
  ->Belt.Array.getBy(({teamId}) => teamId === id)
  ->Belt.Option.map(({thresholds}) => thresholds)
  ->Belt.Option.getWithDefault([||]);
};

/**
 * Components
 */
module UserListItem = {
  [@react.component]
  let make = (~name: string) => {
    <Spread props={"data-testid": "user-list-item"}>
      <li>
        <p className="truncate text-gray-700"> {name |> React.string} </p>
      </li>
    </Spread>;
  };
};

module TeamListItem = {
  [@react.component]
  let make =
      (
        ~id: teamId,
        ~name: string,
        ~users: array(user),
        ~approvers: array(user),
        ~onSelect: unit => unit,
      ) => {
    <Spread props={"data-testid": "team-list-item"}>
      <li className="px-6 py-2">
        <p className="truncate font-semibold"> {name |> React.string} </p>
        <div className="flex justify-between">
          <div className="flex-1 flex flex-col">
            <h3> "Members"->React.string </h3>
            <ul className="flex gap-x-2">
              {users
               ->Belt.Array.slice(~offset=0, ~len=3)
               ->Belt.Array.map(({firstName, lastName, id}) =>
                   <UserListItem
                     key={id->idToString}
                     name={firstName ++ " " ++ lastName}
                   />
                 )
               ->React.array}
            </ul>
          </div>
          <div className="flex-1 flex flex-col items-end">
            <h3> "Approvers"->React.string </h3>
            <ul className="flex gap-x-2">
              {approvers
               ->Belt.Array.slice(~offset=0, ~len=3)
               ->Belt.Array.map(({firstName, lastName, id}) =>
                   <UserListItem
                     key={id->idToString}
                     name={firstName ++ " " ++ lastName}
                   />
                 )
               ->React.array}
              <button
                type_="button"
                onClick={_ => onSelect()}
                className="text-blue-500 font-semibold">
                "Edit"->React.string
              </button>
            </ul>
          </div>
        </div>
      </li>
    </Spread>;
  };
};

module TeamList = {
  [@react.component]
  let make =
      (
        ~teams: array(team),
        ~users: array(user)=[||],
        ~approvalFlows: array(approvalFlow)=[||],
      ) => {
    let (selectedTeam, setSelectedTeam) = React.useState(() => None);

    <Container>
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows" |> React.string}
      </h1>
      <ul className="divide-y">
        {teams
         ->Belt.Array.map(({name, id, userIds}) =>
             <TeamListItem
               key={id->idToString}
               id
               name
               users={getUsersByUserIds(users, userIds)}
               approvers=[||]
               onSelect={() => setSelectedTeam(_ => Some(id))}
             />
           )
         ->React.array}
      </ul>
      {switch (selectedTeam) {
       | Some(id) =>
         let {userIds, _} =
           teams->Belt.Array.getBy(team => team.id === id)->Belt.Option.getExn;
         <TeamApprovalFlow
           id={id->idToString}
           users={getUsersByUserIds(users, userIds)}
           thresholds={getThresholdsByTeamId(approvalFlows, id)}
           onConfirm={() => ()}
           onCancel={() => setSelectedTeam(_ => None)}
         />;
       | _ => React.null
       }}
    </Container>;
  };
};

[@react.component]
let make = () => {
  let (teams, _, teamsError) = useFetcher("/spx-development/contents/teams");
  let (users, _, usersError) = useFetcher("/spx-development/contents/users");
  let (approvalFlows, _, approvalFlowsError) = useLocalApprovalFlows();

  let error =
    switch (teamsError, usersError, approvalFlowsError) {
    | (Some(_), _, _)
    | (_, Some(_), _)
    | (_, _, Some(_)) => true
    | _ => false
    };

  switch (teams, users, approvalFlows, error) {
  | (_, _, _, true) =>
    <p className="text-red-500 my-8">
      {"An error occurred." |> React.string}
    </p>
  | (Some(teams), Some(users), Some(approvalFlows), _) =>
    <TeamList users teams approvalFlows />
  | _ => <p className="text-gray-500 my-8"> {"loading..." |> React.string} </p>
  };
};
