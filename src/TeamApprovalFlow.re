open Types;

[@react.component]
let make =
    (
      ~id: string,
      ~users: array(user),
      ~thresholds: array(threshold),
      ~onCancel,
      ~onConfirm,
    ) => {
  <Container> "TeamApprovalFlow"->React.string </Container>;
};
