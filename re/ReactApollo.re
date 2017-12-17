type hoc = ReasonReact.reactClass => ReasonReact.reactClass;

let mapOptional = (mapper, optional) => switch(optional) {
  | None => None
  | Some(value) => Some(mapper(value))
};

module type Query = {type data; type variables; let query: GraphQLTag.definitions;};

module CreateWrapper = (Query: Query) => {
  type options = {
    .
    "options": {. "fetchPolicy": string, "variables": Js.Null_undefined.t(Query.variables)}
  };
  [@bs.module "react-apollo"] external graphql : (GraphQLTag.definitions, options) => hoc =
    "graphql";
  type props = {. "data": Query.data};
  let component = ReasonReact.statelessComponent("ApolloQueryWrapper");
  let make' = (~data: Query.data, children) => {
    ...component,
    render: (_self) => children(~data)
  };
  let make = (~variables=?, children) => {
    let jsComponent =
      ReasonReact.wrapReasonForJs(
        ~component,
        (props: props) => {
          let data = props##data;
          make'(~data, children)
        }
      );
    let enhanced =
      graphql(
        Query.query,
        {
          "options": {
            "fetchPolicy": "cache-and-network",
            "variables": Js.Null_undefined.from_opt(variables)
          }
        },
        jsComponent
      );
    ReasonReact.wrapJsForReason(~reactClass=enhanced, ~props=Js.Obj.empty(), [||])
  };
};

module CreateMutationWrapper =
       (Query: {type input; type response; let query: GraphQLTag.definitions;}) => {
  type options = {
    .
    "options": {. "fetchPolicy": string, "refetchQueries": Js.Null_undefined.t(array(string))}
  };
  [@bs.module "react-apollo"] external graphql : (GraphQLTag.definitions, options) => hoc =
    "graphql";
  type jsMutationArgs = {. "variables": {. "input": Query.input}};
  type props = {. "mutate": jsMutationArgs => Js.Promise.t({. "data": Query.response})};
  let component = ReasonReact.statelessComponent("ApolloWrapper");
  let make' = (~mutate: Query.input => Js.Promise.t(Query.response), children) => {
    ...component,
    render: (_self) => children(~mutate)
  };
  let make = (~refetchQueries: option(list(string))=?, children) => {
    let jsComponent =
      ReasonReact.wrapReasonForJs(
        ~component,
        (props: props) => {
          let mutate = (variables: Query.input) => {
            let mutate' = props##mutate;
            Js.Promise.(
              mutate'({"variables": {"input": variables}})
              |> then_((data) => data##data |> resolve)
            )
          };
          make'(~mutate, children)
        }
      );
    let enhanced =
      graphql(
        Query.query,
        {
          "options": {
            "fetchPolicy": "network",
            "refetchQueries":
              refetchQueries |> mapOptional(Array.of_list) |> Js.Null_undefined.from_opt
          }
        },
        jsComponent
      );
    ReasonReact.wrapJsForReason(~reactClass=enhanced, ~props=Js.Obj.empty(), [||])
  };
};
