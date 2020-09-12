// Reason doesn't support using props with dashes right now,
// ie: data-id or data-whatever. You can overcome this by
// creating a Spread component
// See https://reasonml.github.io/reason-react/docs/en/adding-data-props

[@react.component]
let make = (~props, ~children) => React.cloneElement(children, props);
