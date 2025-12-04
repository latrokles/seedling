## python implementation of Mary Rose Cook's Little Lisp Interpreter

import argparse
import functools
import pathlib
import os
import typing


def debug(fn):
    @functools.wraps(fn)
    def debug_wrapper(*args, **kwargs):
        print(f'call={fn.__name__}, {args=}, {kwargs=}', end='')
        res = fn(*args, **kwargs)
        print(f'-> {res}')
        return res
    return debug_wrapper


def tokenize(src):
    return (
        src
        .replace('(', ' ( ')
        .replace(')', ' ) ')
        .strip()
        .split()
    )


def parenthesize(tokens, lst=None):
    if lst is None:
        return parenthesize(tokens, [])

    if not tokens:
        return lst.pop()

    token = tokens.pop(0)
    if token == '(':
        lst.append(parenthesize(tokens, []))
        return parenthesize(tokens, lst)

    if token == ')':
        return lst

    return parenthesize(tokens, lst + [categorize(token)])


def categorize(token):
    if parse_int(token) is not None:
        return { 'type': 'literal', 'value': parse_int(token) }

    if parse_float(token) is not None:
        return { 'type': 'literal', 'value': parse_float(token) }

    if token[0] == '"' and token[-1] == '"':
        return { 'type': 'literal', 'value': token[1:-1] }

    return { 'type': 'identifier', 'value': token }


def parse_int(token):
    try:
        return int(token)
    except:
        return None


def parse_float(token):
    try:
        return float(token)
    except:
        return None


def parse(src):
    return parenthesize(tokenize(src))


class Context:
    def __init__(self, scope, parent=None):
        self.scope = scope
        self.parent = parent

    def get(self, identifier):
        if identifier in self.scope.keys():
            return self.scope[identifier]

        if self.parent:
            return self.parent.get(identifier)

        # TODO: nil?

## special forms
def let_form(expr, ctx):
    let_bindings = expr[1]
    let_body = expr[2]

    let_ctx = Context(
        { x[0]['value']: interpret(x[1], ctx) for x in let_bindings },
        ctx
    )
    return interpret(let_body, let_ctx)


def lambda_form(expr, ctx):
    def fn(*args):
        lambda_params = expr[1]
        lambda_body = expr[2]
        lambda_scope = {
            lambda_params[i]['value']: args[i]
            for i in range(len(lambda_params))
        }
        return interpret(lambda_body, Context(lambda_scope, ctx))

    return fn


def if_form(expr, ctx):
    cond, true_expr, false_expr = expr[1:]
    if interpret(cond, ctx):
        return interpret(true_expr, ctx)
    return interpret(false_expr, ctx)


SPECIAL = {
    'lambda': lambda_form,
    'let': let_form,
    'if': if_form
}

LIBRARY = {
    'head': lambda x: x[0],
    'tail': lambda x: x[1:],
    'print': lambda x: print(x),
    'read': lambda _: input(),
    '+': lambda x, y: x + y,
    '-': lambda x, y: x -y,
}


def interpret(expr, ctx=None):
    if not ctx:
        return interpret(expr, Context(LIBRARY))

    if isinstance(expr, list):
        return interpret_list(expr, ctx)

    if expr['type'] == 'identifier':
        return ctx.get(expr['value'])

    return expr['value']


def interpret_list(expr, ctx):
    if expr and expr[0]['value'] in SPECIAL:
        return SPECIAL[expr[0]['value']](expr, ctx)

    evaluated = [interpret(e, ctx) for e in expr]
    if isinstance(evaluated[0], typing.Callable):
        return evaluated[0].__call__(*evaluated[1:])

    return evaluated


def repl():
    while True:
        src = input("> ")
        print(interpret(parse(src)))


def run(pathname):
    src = pathlib.Path(pathname).read_text('utf-8')
    interpret(parse(src))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--debug', help='set debug mode', action='store_true')
    parser.add_argument('--source-file', default=None)
    args = parser.parse_args()

    if not args.source_file:
        repl()
    else:
        run(args.source_file)
