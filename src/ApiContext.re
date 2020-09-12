type options = {baseUrl: string};

let context = React.createContext({baseUrl: ""});

module Provider = {
  let makeProps = (~value, ~children, ()) => {
    "value": value,
    "children": children,
  };
  let make = React.Context.provider(context);
};
