#! /usr/bin/env python

import sys

def has_conditional_effects(task):
    with open(task) as f:
        in_op = False
        for line in f:
            line = line.strip()
            if line == "begin_operator":
                in_op = True
            elif line == "end_operator":
                in_op = False
            elif in_op:
                parts = line.split()
                if len(parts) >= 6 and all(p.lstrip('-').isdigit() for p in parts):
                    print "Task has at least one conditional effect: %s" % line
                    return True
    print "Task has no conditional effects"
    return False

if __name__ == "__main__":
    has_conditional_effects(TASK)
