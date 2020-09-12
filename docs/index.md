# Types

```reasonml
type teamId = string;

type userId = string;

type team = {
  id: teamId,
  name: string,
  userIds: array(userId),
};

type user = {
  id: userId,
  firstName: string,
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
```

# Usage

```reasonml
<TeamApprovalFlows />
```
