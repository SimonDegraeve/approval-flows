open Types;

module Form = {
  [@react.component]
  let make =
      (
        ~users: array(user),
        ~onChange: threshold => unit,
        ~min: option(float)=?,
        ~max: option(float)=?,
        ~userId: option(userId)=?,
      ) => {
    let (min, setMin) = React.useState(() => min);
    let onMinChange = event => {
      let value = event->ReactEvent.Form.target##value;
      setMin(_ => value);
    };

    let (max, setMax) = React.useState(() => max);
    let onMaxChange = event => {
      let value = event->ReactEvent.Form.target##value;
      setMax(_ => value);
    };

    let (userId, setUserId) = React.useState(() => userId);
    let onUserChange = event => {
      let value = event->ReactEvent.Form.target##value;
      setUserId(_ => value);
    };

    let onSubmit = event => {
      event->ReactEvent.Form.preventDefault;
      onChange(
        {
          min: min->Belt.Option.getWithDefault(0.0),
          max: max->Belt.Option.getWithDefault(infinity),
          userId: userId->Belt.Option.getExn,
        }: threshold,
      );
    };

    <form className="max-w-sm" onSubmit>
      <div className="mb-4">
        <label
          className="block text-gray-700 text-sm font-bold mb-2" htmlFor="min">
          "From"->React.string
        </label>
        <input
          className="appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          id="min"
          required=true
          type_="number"
          min="0"
          value={
            min
            ->Belt.Option.map(Js.Float.toString)
            ->Belt.Option.getWithDefault("")
          }
          onChange=onMinChange
        />
      </div>
      <div className="mb-4">
        <label
          className="block text-gray-700 text-sm font-bold mb-2" htmlFor="max">
          "To"->React.string
        </label>
        <input
          className="appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          id="max"
          type_="number"
          min=?{min->Belt.Option.map(Js.Float.toString)}
          value={
            max
            ->Belt.Option.map(Js.Float.toString)
            ->Belt.Option.getWithDefault("")
          }
          onChange=onMaxChange
        />
      </div>
      <div className="mb-6">
        <label
          className="block text-gray-700 text-sm font-bold mb-2" htmlFor="max">
          "User"->React.string
        </label>
        <select
          required=true
          className="appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          id="user"
          value={
            userId
            ->Belt.Option.map(idToString)
            ->Belt.Option.getWithDefault("")
          }
          onChange=onUserChange>
          <option value="" />
          {{
             users->Belt.Array.map(({firstName, lastName, id}) =>
               <option value={id->idToString} key={id->idToString}>
                 {{
                    firstName ++ " " ++ lastName;
                  }
                  ->React.string}
               </option>
             );
           }
           ->React.array}
        </select>
      </div>
      <div className="flex items-center justify-between">
        <button
          className="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline"
          type_="submit">
          "Confirm"->React.string
        </button>
      </div>
    </form>;
  };
};

module ThresholdListItem = {
  [@react.component]
  let make = (~min: float, ~max: float, ~user: user, ~onDelete) => {
    <li className="flex justify-between items-center">
      <p className="truncate text-gray-700 py-4">
        {(
           "From "
           ++ min->Js.Float.toString
           ++ " to "
           ++ max->Js.Float.toString
           ++ {j|€ needs approval by |j}
           ++ user.firstName
           ++ " "
           ++ user.lastName
         )
         ->React.string}
      </p>
      <button
        onClick={_ => onDelete()}
        className="bg-gray-200 hover:bg-gray-300 font-bold px-2 py-1 rounded focus:outline-none focus:shadow-outline text-sm"
        type_="button">
        "Delete"->React.string
      </button>
    </li>;
  };
};

[@react.component]
let make =
    (
      ~teamName: string,
      ~users: array(user),
      ~thresholds: array(threshold),
      ~onClose,
      ~onConfirm,
    ) => {
  let (formKey, setFormKey) = React.useState(() => 1);
  let onCreateThreshold = threshold => {
    onConfirm(thresholds->Js.Array.concat([|threshold|]));
    setFormKey(key => key + 1);
  };

  let onDeleteThreshold = index => {
    onConfirm(thresholds->Belt.Array.keepWithIndex((_, i) => i !== index));
    setFormKey(key => key + 1);
  };

  <Container>
    <div className="flex justify-between">
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows for " ++ teamName |> React.string}
      </h1>
      <button
        onClick={_ => onClose()}
        className="bg-gray-200 hover:bg-gray-300 font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline"
        type_="button">
        "Close"->React.string
      </button>
    </div>
    <ul className="divide-y mb-6">
      {thresholds
       ->Belt.Array.mapWithIndex((index, {min, max, userId}) =>
           <ThresholdListItem
             key={index->Js.Int.toString}
             min
             max
             user={getUserByUserId(users, userId)->Belt.Option.getExn}
             onDelete={() => onDeleteThreshold(index)}
           />
         )
       ->React.array}
    </ul>
    <Form users onChange=onCreateThreshold key={formKey->Js.Int.toString} />
  </Container>;
};
