## python implementation of Mary Rose Cook's Little Lisp Interpreter

import pprint

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


if __name__ == '__main__':
    pprint.pprint(parse('((lambda (x) x) "Lisp")'))
