let mapOptional = (mapper, optional) => switch(optional) {
  | None => None
  | Some(value) => Some(mapper(value))
};

type hoc = ReasonReact.reactClass => ReasonReact.reactClass;

module type Query = {type data; type variables; let query: GraphQLTag.definitions;};

module CreateWrapper = (Query: Query) => {
  type children = (~data: Query.data) => ReasonReact.reactElement;
  type props = {. "data": Query.data, "children": children};
  type options = {
    .
    "options":
      {. "variables": option(Query.variables)} =>
      {. "fetchPolicy": string, "variables": Js.Null_undefined.t(Query.variables)}
  };
  [@bs.module "react-apollo"] external graphql : (GraphQLTag.definitions, options) => hoc =
    "graphql";
  let component = ReasonReact.statelessComponent("ApolloQueryWrapper");
  let make' = (~data: Query.data, children: children) => {...component, render: (_self) => children(~data)};
  let jsComponent =
    ReasonReact.wrapReasonForJs(
      ~component,
      (props: props) => {
        let data = props##data;
        make'(~data, props##children)
      }
    );
  let enhanced =
    graphql(
      Query.query,
      {
        "options": (props) => {
          "fetchPolicy": "cache-and-network",
          "variables": Js.Null_undefined.from_opt(props##variables)
        }
      },
      jsComponent
    );
  let make = (~variables: option(Query.variables)=?, children: children) =>
    ReasonReact.wrapJsForReason(~reactClass=enhanced, ~props={"variables": variables}, children);
};

module CreateMutationWrapper =
       (Query: {type input; type response; let query: GraphQLTag.definitions;}) => {
  type options = {
    .
    "options":
      {. "refetchQueries": option(list(string))} =>
      {. "fetchPolicy": string, "refetchQueries": Js.Null_undefined.t(array(string))}
  };
  [@bs.module "react-apollo"] external graphql : (GraphQLTag.definitions, options) => hoc =
    "graphql";
  type jsMutationArgs = {. "variables": Query.input};
  type mutate = Query.input => Js.Promise.t(Query.response);
  type children  = (~mutate: mutate) => ReasonReact.reactElement;
  type props = {
    .
    "mutate": jsMutationArgs => Js.Promise.t({. "data": Query.response}),
    "children": children
  };
  let component = ReasonReact.statelessComponent("ApolloWrapper");
  let make' = (~mutate: mutate, children) => {...component, render: (_self) => children(~mutate)};
  let jsComponent =
    ReasonReact.wrapReasonForJs(
      ~component,
      (props: props) => {
        let mutate = (variables: Query.input) => {
          let mutate' = props##mutate;
          Js.Promise.(
            mutate'({"variables": variables}) |> then_((data) => data##data |> resolve)
          )
        };
        make'(~mutate, props##children)
      }
    );
  let enhanced =
    graphql(
      Query.query,
      {
        "options": (props) => {
          "fetchPolicy": "network",
          "refetchQueries":
            props##refetchQueries
            |> mapOptional(Array.of_list)
            |> Js.Null_undefined.from_opt
        }
      },
      jsComponent
    );
  let make = (~refetchQueries: option(list(string))=?, children: children) =>
    ReasonReact.wrapJsForReason(
      ~reactClass=enhanced,
      ~props={"refetchQueries": refetchQueries},
      children
    );
};
