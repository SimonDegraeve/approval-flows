[@react.component]
let make = (~children) => {
  <div
    className="h-screen w-screen absolute top-0 left-0 flex justify-center items-center bg-blue-100">
    <div
      className="max-w-4xl w-full rounded overflow-hidden shadow-lg p-4 bg-white">
      children
    </div>
  </div>;
};
