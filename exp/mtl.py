import argparse
import dataclasses


@dataclasses.dataclass




def repl():
    pass

def run(src):
    pass



if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--debug', help='set debug mode', action='store_true')
    parser.add_argument('--source-file', default=None)
    args = parser.parse_args()

    if not args.source_file:
        repl()
    else:
        run(args.source_file)
