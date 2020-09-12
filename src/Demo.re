module App = {
  [@react.component]
  let make = () => {
    let apiOptions: ApiContext.options = {
      baseUrl: "https://s3-eu-west-1.amazonaws.com/",
    };

    <div className="h-screen flex justify-center items-center">
      <div className="max-w-xl w-full rounded overflow-hidden shadow-lg p-4">
        <ApiContext.Provider value=apiOptions>
          <TeamApprovalFlows />
        </ApiContext.Provider>
      </div>
    </div>;
  };
};

[%bs.raw "require('./tailwind.css')"];
ReactDOMRe.renderToElementWithId(<App />, "root");
