# bs-reason-apollo
[![Build Status](https://travis-ci.org/Astrocoders/bs-reason-apollo.svg?branch=master)](https://travis-ci.org/Astrocoders/bs-reason-apollo)

Initially inspired in https://github.com/FormidableLabs/seattlejsconf-app/blob/master/re/types/ReactApollo.re
But now with a more sugared usage with function as child.


## Difference from reason-apollo official
This allows you to use an React Apollo Client that you already have in JS so you can gradually change to Reason.
Setup the same way you would do for React Apollo and just plug it in.

## Install

```
yarn add bs-reason-apollo
```

Update your bs-config.json
```js
  "bs-dependencies": ["reason-react", "bs-reform", "bs-reason-apollo"],
```

## ReactApollo.CreateWrapper
As you have your ApolloProvider somewhere in the top of your React JS tree you are already covered there.
So now to use it in Apollo create a query definition module for you query:

```reason
/* re/SignInQueryGql.re */
open BsReasonApollo;

let query = GraphQLTag.gql({|
  query SignInQuery {
    currentUser {
      id
      email
    }
  }
|});

module Types = {
  type user = {. "id": string, "email": string};
  type error = {. "message": string};
  /* You must always have this data type with loading and error, it's what the HOC gives you */
  type data = {
    .
    "loading": Js.boolean,
    "error": Js.null_undefined(error),
    /* Our response */
    "currentUser": Js.null_undefined(user)
  };
};

type data = Types.data;

/* Define any Js.t variables that you query need here, if you don't use just declare it */
type variables;

type response = Types.user;
```

Now in your actually component:

```reason
open BsReasonpollo;

module SignInQueryWrapper = ReactApollo.CreateWrapper(SignInQueryGql);

...
let make = (_children) => {
  ...,
  render: (_self) =>
    <SignInQueryWrapper>
    ...((~data) =>
        switch (
          Js.to_bool(data##loading),
          Js.Null_undefined.to_opt(data##error),
          Js.Null_undefined.to_opt(data##currentUser)
        ) {
        | (true, _, _) => <FullLoading />
        | (false, _, Some(user)) =>
          <Welcome user />
        | (false, Some(error), _) => <Whoops name=error##message />
        | (false, None, _) =>
          <KeyboardAwareScrollView>
            <Wrapper>
              <Header>
                <Logo
                  source=Image.(
                           Required(Packager.require("../../../src/public/img/logo-calendar.png"))
                         )
                />
              </Header>
              <ContentWrapper
                contentContainerStyle=Style.(
                                        style([
                                          paddingVertical(Pt(40.)),
                                          justifyContent(SpaceAround)
                                        ])
                                      )>
                <SignInForm />
              </ContentWrapper>
            </Wrapper>
          </KeyboardAwareScrollView>
        }
      )
    </SignInQueryWrapper>
}
```

## ReactApollo.CreateMutationWrapper

Define the mutation module:

```reason
/* re/SignInMutationGql.re */
open BsReasonpollo;

let query = GraphQLTag.gql({|
  mutation SignInQuery($input: SignInInput!) {
    signIn(input: $input) {
      error
      token
    }
  }
|});

module Types = {
  type input = {. "password": string, "email": string};
  type signIn = {. "error": Js.null_undefined(string), "token": Js.null_undefined(string)};
};

/* Needed for mutations, it'll be probably `variables` in the next release */
type input = Types.input;

/* Mutation response */
type response = {. "signIn": Types.signIn};
```

```reason
open BsReasonpollo;

/* Mutation wrapper */
module SignInMutationWrapper = ReactApollo.CreateMutationWrapper(SignInQueryGql);

/* https://github.com/Astrocoders/reform */
module SignInForm = ReForm.Create(SignInFormParams);

let convertInputToJs: SignInFormParams.state => SignInMutationGql.Types.signInInput =
  (values) => {"password": values.password, "email": values.email};

let handleSubmit = (mutate, values, setSubmitting) =>
  values
  |> convertToJs
  |> mutate
  |> Js.Promise.(
       then_(
         (payload) =>
           (
             switch (Js.Null_undefined.to_opt(payload##signIn##error)) {
             | Some(error) =>
               Alert.alert(~title="Something went wrong", ~message=error, ());
               setSubmitting(false)
             | None =>
               RouterActions.home(~actionType=`replace);
               let _ =
                 Utils.UserSession.setToken(
                   Utils.get(Js.Null_undefined.to_opt(payload##signIn##token), "")
                 );
               ignore()
             }
           )
           |> resolve
       )
     )
  |> ignore;

/* A little abstraction to make function as child composition hurt a bit less */
let enhanced = (mapper) => {
  <SignInMutationWrapper>
    ...((~mutate) => (
      <SignInForm
        initialValues={etc}
         onSubmit=(
           (values, ~setSubmitting, ~setError as _) =>
             handleSubmit(mutate, values, setSubmitting)
         )
      >
        ...mapper
      </SignInForm>
    ))
  </SignInMutationWrapper>
};

let component = 
```

## Demo
WIP
