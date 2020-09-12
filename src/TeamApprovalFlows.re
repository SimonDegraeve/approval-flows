open Types;

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

module UserListItem = {
  [@react.component]
  let make = (~id: string, ~name: string) => {
    <li>
      <p className="truncate text-gray-700"> {name |> React.string} </p>
    </li>;
  };
};

module TeamListItem = {
  [@react.component]
  let make =
      (
        ~id: string,
        ~name: string,
        ~users: array(user),
        ~approvers: array(user),
      ) => {
    <li className="px-6 py-2">
      <p className="truncate font-semibold"> {name |> React.string} </p>
      <ul className="flex gap-x-2">
        {users
         ->Belt.Array.slice(~offset=0, ~len=3)
         ->Belt.Array.map(({firstName, lastName, id}) =>
             <UserListItem
               name={firstName ++ " " ++ lastName}
               id={id->idToString}
               key={id->idToString}
             />
           )
         ->React.array}
      </ul>
    </li>;
  };
};

module TeamList = {
  [@react.component]
  let make = (~teams: array(teamWithUsers)) => {
    <>
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows" |> React.string}
      </h1>
      <ul className="divide-y">
        {teams
         ->Belt.Array.map(({name, id, users}) =>
             <TeamListItem
               name
               users
               approvers=[||]
               id={id->idToString}
               key={id->idToString}
             />
           )
         ->React.array}
      </ul>
    </>;
  };
};

[@react.component]
let make = () => {
  let (teams, _, teamsError) = useFetcher("/spx-development/contents/teams");
  let (users, _, usersError) = useFetcher("/spx-development/contents/users");

  let getUserById = (users: array(user), id: userId) =>
    users->Belt.Array.getBy(user => user.id === id);

  let getUsersByIds = (users: array(user), ids: array(userId)) =>
    ids->Belt.Array.keepMap(id => getUserById(users, id));

  switch (teams, users, teamsError, usersError) {
  | (Some(teams), Some(users), _, _) =>
    <TeamList
      teams={
        teams
        |> Js.Array.map(({id, name, userIds}) =>
             {id, name, users: getUsersByIds(users, userIds)}
           )
      }
    />
  | (_, _, Some(_), _)
  | (_, _, _, Some(_)) =>
    <p className="text-red-500 my-8">
      {"An error occurred." |> React.string}
    </p>
  | _ => <p className="text-gray-500 my-8"> {"loading..." |> React.string} </p>
  };
};
