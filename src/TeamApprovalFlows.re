open Types;

type fetchResult('data, 'error) = {
  data: Js.Nullable.t('data),
  error: Js.Nullable.t('error),
};

[@bs.module "swr"]
external useSwr: (string, 'fn) => fetchResult('data, 'error) = "default";

let useTeams = () => {
  let path = "/spx-development/contents/teams";

  let apiOptions = React.useContext(ApiContext.context);

  let fetcher = url =>
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

module TeamListItem = {
  [@react.component]
  let make = (~team: Types.team) => {
    <li className="px-6 py-2">
      <p className="truncate"> {team.name |> React.string} </p>
    </li>;
  };
};

module TeamList = {
  [@react.component]
  let make = (~teams: array(Types.team)) => {
    <>
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows" |> React.string}
      </h1>
      <ul className="divide-y">
        {teams
         |> Array.map(team => <TeamListItem team key={team.id->idToString} />)
         |> React.array}
      </ul>
    </>;
  };
};

[@react.component]
let make = () => {
  let teams = useTeams();
  switch (teams) {
  | (Some(teams), _, _) => <TeamList teams />
  | (_, _, Some(_)) =>
    <p className="text-red-500"> {"An error occurred." |> React.string} </p>
  | _ => <p className="text-gray-500"> {"loading..." |> React.string} </p>
  };
};
