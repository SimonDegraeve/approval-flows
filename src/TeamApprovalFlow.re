open Types;
open Components;

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
        <Input
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
        <Input
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
        <Select
          required=true
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
        </Select>
      </div>
      <div className="flex items-center justify-between">
        <Button
          className="bg-blue-500 hover:bg-blue-700 text-white" type_="submit">
          "Confirm"->React.string
        </Button>
      </div>
    </form>;
  };
};

module ThresholdListItem = {
  [@react.component]
  let make =
      (~id, ~min: float, ~max: float, ~user: user, ~onDelete, ~onSelect) => {
    <Spread props={"data-testid": "threshold-list-item-" ++ id}>
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
        <div>
          <Button
            onClick={_ => onSelect()}
            className="text-sm pl-2 pr-2 pt-1 pb-1 mr-2"
            type_="button">
            "Edit"->React.string
          </Button>
          <Button
            onClick={_ => onDelete()}
            className="text-sm pl-2 pr-2 pt-1 pb-1"
            type_="button">
            "Delete"->React.string
          </Button>
        </div>
      </li>
    </Spread>;
  };
};

type state = {
  thresholds: array(threshold),
  selectedThreshold: option(int),
};

type action =
  | CreateThreshold(threshold)
  | UpdateThreshold(threshold, int)
  | DeleteThreshold(int)
  | SelectThreshold(int);

[@react.component]
let make =
    (
      ~teamName: string,
      ~users: array(user),
      ~thresholds: array(threshold),
      ~onClose,
      ~onChange,
    ) => {
  let (formKey, setFormKey) = React.useState(() => 1);

  // Handle thresholds state
  let (state, send) =
    React.useReducer(
      (state, action) =>
        switch (action) {
        | CreateThreshold(threshold) => {
            thresholds: state.thresholds->Belt.Array.concat([|threshold|]),
            selectedThreshold: None,
          }
        | UpdateThreshold(threshold, index) => {
            thresholds:
              state.thresholds
              ->Belt.Array.mapWithIndex((currentIndex, currentThreshold) =>
                  index === currentIndex ? threshold : currentThreshold
                ),
            selectedThreshold: None,
          }
        | DeleteThreshold(index) => {
            thresholds:
              state.thresholds
              ->Belt.Array.keepWithIndex((_, i) => i !== index),
            selectedThreshold: None,
          }
        | SelectThreshold(index) => {
            thresholds: state.thresholds,
            selectedThreshold: Some(index),
          }
        },
      {thresholds, selectedThreshold: None},
    );

  // Notify parent on change
  React.useEffect1(
    () => {
      if (state.thresholds !== thresholds) {
        Js.log("ok");
        onChange(state.thresholds);
        setFormKey(key => key + 1);
      };
      None;
    },
    [|state.thresholds|],
  );

  // Reset form on threshold selection
  React.useEffect1(
    () => {
      setFormKey(key => key + 1);
      None;
    },
    [|state.selectedThreshold|],
  );

  let selectedThreshold =
    state.selectedThreshold
    ->Belt.Option.flatMap(index => state.thresholds->Belt.Array.get(index));

  <Container>
    <div className="flex justify-between">
      <h1 className="text-blue-800 mb-4 text-lg font-semibold">
        {"Teams approval flows for " ++ teamName |> React.string}
      </h1>
      <Button onClick={_ => onClose()} type_="button">
        "Close"->React.string
      </Button>
    </div>
    <Spread props={"data-testid": "team-approval-flow"}>
      <ul className="divide-y mb-6">
        {state.thresholds
         ->Belt.Array.mapWithIndex((index, {min, max, userId}) =>
             <ThresholdListItem
               key={index->Js.Int.toString}
               id={index->Js.Int.toString}
               min
               max
               user={getUserByUserId(users, userId)->Belt.Option.getExn}
               onDelete={() => send(DeleteThreshold(index))}
               onSelect={() => send(SelectThreshold(index))}
             />
           )
         ->React.array}
      </ul>
    </Spread>
    <Form
      users
      min=?{
        switch (selectedThreshold) {
        | None =>
          // Use previous max value as lower bound for new threshold
          state.thresholds
          ->Belt.Array.get(thresholds->Belt.Array.length - 1)
          ->Belt.Option.map(({max}) => max)
        | Some(selectedThreshold) => Some(selectedThreshold.min)
        }
      }
      max=?{
        switch (selectedThreshold) {
        | None => None
        | Some(selectedThreshold) => Some(selectedThreshold.max)
        }
      }
      userId=?{
        switch (selectedThreshold) {
        | None => None
        | Some(selectedThreshold) => Some(selectedThreshold.userId)
        }
      }
      onChange={threshold => {
        switch (state.selectedThreshold) {
        | None => send(CreateThreshold(threshold))
        | Some(index) => send(UpdateThreshold(threshold, index))
        }
      }}
      key={formKey->Js.Int.toString}
    />
  </Container>;
};
