module Spread = {
  // Reason doesn't support using props with dashes right now,
  // ie: data-id or data-whatever. You can overcome this by
  // creating a Spread component
  // See https://reasonml.github.io/reason-react/docs/en/adding-data-props

  [@react.component]
  let make = (~props, ~children) => React.cloneElement(children, props);
};

module Container = {
  [@react.component]
  let make = (~children) => {
    <div
      className="h-screen w-screen absolute top-0 left-0 flex justify-center items-center bg-blue-100">
      <div
        className="max-w-4xl w-full rounded overflow-hidden shadow-lg p-4 bg-white">
        children
      </div>
    </div>;
  };
};

module Button = {
  [@react.component]
  let make = (~children, ~type_, ~onClick=?, ~className="") => {
    <button
      type_
      ?onClick
      className={
        className
        ++ " bg-gray-200 hover:bg-gray-300 font-bold rounded focus:outline-none focus:shadow-outline py-2 px-4"
      }>
      children
    </button>;
  };
};

module Input = {
  [@react.component]
  let make =
      (
        ~id=?,
        ~type_=?,
        ~onChange=?,
        ~value=?,
        ~min=?,
        ~required=?,
        ~className="",
      ) => {
    <input
      className={
        className
        ++ "appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
      }
      ?id
      ?type_
      ?min
      ?value
      ?required
      ?onChange
    />;
  };
};

module Select = {
  [@react.component]
  let make =
      (~id=?, ~onChange=?, ~value=?, ~required=?, ~className="", ~children) => {
    <select
      className={
        className
        ++ "appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
      }
      ?id
      ?value
      ?required
      ?onChange>
      children
    </select>;
  };
};
