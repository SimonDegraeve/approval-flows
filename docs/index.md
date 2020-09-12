# Approval flows - Senior Frontend Remote Case Study

## Types

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
  userId: userId,
};

type approvalFlow = {
  teamId: teamId,
  thresholds: array(threshold),
};
```

## Usage

The default component handles data fetching automatically. You must wrap it inside an `ApiContextProvider` in order to configure the api options.

```reasonml
module App = {
  [@react.component]
  let make = () => {
    let apiOptions: ApiContext.options = {
      baseUrl: "https://s3-eu-west-1.amazonaws.com/",
    };

    <ApiContext.Provider value=apiOptions> <TeamList /> </ApiContext.Provider>;
  };
};
```

You can also use the component without data fetching. Juts use the exported `TeamList.TeamList` component.

```reasonml
module App = {
  [@react.component]
  let make = () => {
    <TeamList.TeamList
      teams=[|
        {
          id: TeamId("TEAM1"),
          name: "Marketing",
          users: [|
            {
              id: UserId("USR1"),
              firstName: "Eugene",
              lastName: "Tran",
              email: "eugene.tran@spendesk.com",
            },
          |],
          approvers: [|
            {
              id: UserId("USR1"),
              firstName: "Eugene",
              lastName: "Tran",
              email: "eugene.tran@spendesk.com",
            },
          |],
          thresholds: [|{min: 0.0, max: 500.0, userId: UserId("USR1")}|],
        },
      |]
    />;
  };
};
```

## Conception

The project is mostly divided into two main components:

- `TeamList` fetches the data (users, teams and approval flows) and displays a list of teams and is the entry point to edit some approval flow. NB: You can also uses the `TeamList.TeamList` component in order to handle the data fetching by yourself.

- `TeamApprovalFlow` displays the list of thresholds set for a team and is the entry point to create/edit/delete thresholds. This component uses a reducer in order to manage the different actions and renders a single CRUD form.

```reasonml
type state = {
  thresholds: array(threshold),
  selectedThreshold: option(int),
};

type action =
  | CreateThreshold(threshold)
  | UpdateThreshold(threshold, int)
  | DeleteThreshold(int)
  | SelectThreshold(int);
```

The project uses `LocalStorage` in order to save the approval flows locally, it can be easily swap for an api call (see `useLocalApprovalFlows()` hook).

I decided to use [`useSwr()`](https://swr.vercel.app/) hook to fetch the data from the api. I enjoy the caching strategy provided by this library without much friction with the rest of the code.

I didn't use a form library because I didn't want to do to many things "magically". Of course in a real world I would have used one.

I didn't use `redux` because since we have hooks and Suspense I rarely need the extra functionalities provided by redux (time travelling, global stores, etc). I usually use a lib with Suspense for data fetching and `useReducer` hook for complex components and a simple singleton instead of a global store.

I used `reasonml` because I enjoy the confidence of a sound typed language and I used to work with a backend team coding in Scala. Having both front and back sharing the same vocabulary (functional programming) helped us communicate. I also worked a lot with FlowType.

I took some arbitrary decisions concerning the stories, in a real world I would have asked the PO some questions to ensure the following points are clear:

- Can the component be directly accessed by an URL ? or where is it gonna be mounted in the tree? (to know how to handle error handling, data fetching, etc)
- Is there any pagination for the list of team ?
- Can the thresholds for an approval flow overlap each other ?
- Is there any maximum number of thresholds allowed ?
- ...

What I would also have done in a real world development:

- use a router
- use `I18N` library
- use form library
- ensure `ARIA` accessibility
- ensure a nice design
- automated test in browser (eg: cypress)
- CI/CD to test, generate changelog, bump versions and deploy

## TODO

The HR informed me it was a 4h exercise but the instructions inside the PDF said 8h. So I stopped coding after 4:00 and I spend an extra 45min writing this doc. So here is the list of remaining things to do:

- [ ] A user doesn't appear twice in an approval scheme
- [ ] test: A user doesn't appear twice in an approval scheme
- [ ] Thresholds don't overlap each other
- [ ] test: Thresholds don't overlap each other
- [ ] test: Restore thresholds locally (need to spy on localStorage)
- [ ] test: Save thresholds locally (need to spy on localStorage)
- [ ] Handle error in CRUD form
- [ ] Improve approval flows display (use "Up to" when min of threshold is 0 and "Above" and max of threshold is infinity)
- [ ] test: Reset form after being used
