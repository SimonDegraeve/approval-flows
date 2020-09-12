module App = {
  [@react.component]
  let make = () => {
    let apiOptions: ApiContext.options = {
      baseUrl: "https://s3-eu-west-1.amazonaws.com/",
    };

    <ApiContext.Provider value=apiOptions> <TeamList /> </ApiContext.Provider>;
  };
};

[%bs.raw "require('./tailwind.css')"];
ReactDOMRe.renderToElementWithId(<App />, "root");
