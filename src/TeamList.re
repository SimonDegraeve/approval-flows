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

[@bs.module "swr"] external revalidate: string => unit = "mutate";

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

let approvalFlowsKey = "approval-flows";

let saveApprovalFlows = (approvalFlows: array(approvalFlow)) => {
  setItem(approvalFlowsKey, approvalFlows->Obj.magic->Js.Json.stringify);
  revalidate(approvalFlowsKey);
};

let useLocalApprovalFlows = () => {
  let fetcher = () =>
    (
      try(Some(Js.Json.parseExn(getItem(approvalFlowsKey))->Obj.magic)) {
      | _ => None
      }
    )
    ->Js.Nullable.fromOption
    ->Js.Promise.resolve;

  let {data, error} = useSwr(approvalFlowsKey, fetcher);

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
        ~name: string,
        ~users: array(user),
        ~usersVisible: int=3,
        ~approversVisible: int=3,
        ~approvers: array(user),
        ~onSelect: unit => unit,
      ) => {
    <Spread props={"data-testid": "team-list-item"}>
      <li className="px-6 py-2">
        <p className="truncate font-semibold"> {name |> React.string} </p>
        <div className="flex justify-between">
          <div className="flex-1 flex flex-col">
            <h3> "Members"->React.string </h3>
            <ul className="flex gap-x-3">
              {users
               ->Belt.Array.slice(~offset=0, ~len=usersVisible)
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
            <ul className="flex gap-x-3">
              {approvers
               ->Belt.Array.slice(~offset=0, ~len=approversVisible)
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
                className="bg-gray-200 hover:bg-gray-300 font-bold px-2 py-1 rounded focus:outline-none focus:shadow-outline text-sm">
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
        ~onApprovalFlowsChange: array(approvalFlow) => unit=_ => (),
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
               name
               users={getUsersByUserIds(users, userIds)}
               approvers={
                 getThresholdsByTeamId(approvalFlows, id)
                 ->Belt.Array.map(({userId}) => userId)
                 ->getUsersByUserIds(users, _)
               }
               onSelect={() => setSelectedTeam(_ => Some(id))}
             />
           )
         ->React.array}
      </ul>
      {switch (selectedTeam) {
       | Some(id) =>
         let {userIds, name} =
           teams->Belt.Array.getBy(team => team.id === id)->Belt.Option.getExn;
         <TeamApprovalFlow
           teamName=name
           users={getUsersByUserIds(users, userIds)}
           thresholds={getThresholdsByTeamId(approvalFlows, id)}
           onConfirm={thresholds =>
             onApprovalFlowsChange(
               approvalFlows
               ->Belt.Array.keep(({teamId}) => teamId !== id)
               ->Belt.Array.concat([|{teamId: id, thresholds}|]),
             )
           }
           onClose={() => setSelectedTeam(_ => None)}
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
    <TeamList
      users
      teams
      approvalFlows
      onApprovalFlowsChange=saveApprovalFlows
    />
  | _ => <p className="text-gray-500 my-8"> {"loading..." |> React.string} </p>
  };
};
