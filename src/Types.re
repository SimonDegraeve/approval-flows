// Id helper to force type conversion to string (eg: use as a `key` prop for a React component)
external idToString: 'id => string = "%identity";

[@unboxed]
type teamId =
  | TeamId(string);

[@unboxed]
type userId =
  | UserId(string);

type team = {
  id: teamId,
  name: string,
  [@bs.as "users"]
  userIds: array(userId),
};

type user = {
  id: userId,
  [@bs.as "first_name"]
  firstName: string,
  [@bs.as "last_name"]
  lastName: string,
  email: string,
};

type threshold = {
  min: float,
  max: float,
  userId,
};

type approvalFlow = {
  teamId,
  thresholds: array(threshold),
};

/**
 * Data conversion utils
 */
let getUserByUserId = (users: array(user), id: userId) =>
  users->Belt.Array.getBy(user => user.id === id);

let getUsersByUserIds = (users: array(user), ids: array(userId)) =>
  ids->Belt.Array.keepMap(id => getUserByUserId(users, id));

let getThresholdsByTeamId =
    (approvalFlows: array(approvalFlow), id: teamId): array(threshold) => {
  approvalFlows
  ->Belt.Array.getBy(({teamId}) => teamId === id)
  ->Belt.Option.map(({thresholds}) => thresholds)
  ->Belt.Option.getWithDefault([||]);
};
