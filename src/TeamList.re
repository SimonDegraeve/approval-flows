open Types;
open Components;

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
  let make = (~id: userId, ~name: string) => {
    <Spread props={"data-testid": "user-list-item-" ++ id->idToString}>
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
        ~usersVisible: int=3,
        ~approversVisible: int=3,
        ~approvers: array(user),
        ~onSelect: unit => unit,
      ) => {
    <Spread props={"data-testid": "team-list-item-" ++ id->idToString}>
      <li className="px-6 py-2 cursor-pointer" onClick={_ => onSelect()}>
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
                     id
                     name={firstName ++ " " ++ lastName}
                   />
                 )
               ->React.array}
            </ul>
          </div>
          {approvers->Belt.Array.length > 0
             ? <div className="flex-1 flex flex-col items-end">
                 <h3> "Approvers"->React.string </h3>
                 <ul className="flex gap-x-3">
                   {approvers
                    ->Belt.Array.slice(~offset=0, ~len=approversVisible)
                    ->Belt.Array.map(({firstName, lastName, id}) =>
                        <UserListItem
                          key={id->idToString}
                          id
                          name={firstName ++ " " ++ lastName}
                        />
                      )
                    ->React.array}
                 </ul>
               </div>
             : React.null}
        </div>
      </li>
    </Spread>;
  };
};

// Dumb component: api agnostic
module TeamList = {
  type team = {
    id: teamId,
    name: string,
    users: array(user),
    approvers: array(user),
    thresholds: array(threshold),
  };

  [@react.component]
  let make =
      (
        ~teams: array(team),
        ~onApprovalFlowsChange: (teamId, array(threshold)) => unit=(_, _) =>
                                                                    (),
      ) => {
    let (selectedTeam, setSelectedTeam) = React.useState(() => None);

    <Container>
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows" |> React.string}
      </h1>
      <Spread props={"data-testid": "team-list"}>
        <ul className="divide-y">
          {teams
           ->Belt.Array.map(({name, id, users, approvers}) =>
               <TeamListItem
                 key={id->idToString}
                 id
                 name
                 users
                 approvers
                 onSelect={() => setSelectedTeam(_ => Some(id))}
               />
             )
           ->React.array}
        </ul>
      </Spread>
      {switch (selectedTeam) {
       | Some(id) =>
         let {users, name, thresholds} =
           teams->Belt.Array.getBy(team => team.id === id)->Belt.Option.getExn;
         <TeamApprovalFlow
           teamName=name
           users
           thresholds
           onChange={thresholds => onApprovalFlowsChange(id, thresholds)}
           onClose={() => setSelectedTeam(_ => None)}
         />;
       | _ => React.null
       }}
    </Container>;
  };
};

// Connected component: handle data fetching and data normalization
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
      teams={
        // Normalize data
        teams->Belt.Array.map(({id, name, userIds}) =>
          (
            {
              id,
              name,
              users: getUsersByUserIds(users, userIds),
              approvers:
                getThresholdsByTeamId(approvalFlows, id)
                ->Belt.Array.map(({userId}) => userId)
                ->getUsersByUserIds(users, _),
              thresholds: getThresholdsByTeamId(approvalFlows, id),
            }: TeamList.team
          )
        )
      }
      onApprovalFlowsChange={(id, thresholds) =>
        saveApprovalFlows(
          approvalFlows
          ->Belt.Array.keep(({teamId}) => teamId !== id)
          ->Belt.Array.concat([|{teamId: id, thresholds}|]),
        )
      }
    />
  | _ => <p className="text-gray-500 my-8"> {"loading..." |> React.string} </p>
  };
};
