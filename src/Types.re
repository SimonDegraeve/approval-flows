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
  userId: array(userId),
};

type approvalScheme = {
  teamId,
  thresholds: array(threshold),
};
