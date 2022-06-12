#!/usr/bin/python

import os;

print("Content-type: text/html\r\n\r\n")

print ("<font size=+1>Environment</font><\br>")

for param in os.environ.keys():
   print ("<b>%20s</b>: %s<\b>" % (param, os.environ[param]))