#/bin/python2
import cgi, cgitb
import sys
from time import sleep

print "\r\n\r\n"

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')
nick_name  = form.getvalue('nick_name')

print "<html>"
print "<head>"
print "<title>Hello - Second CGI Program</title>"
print "</head>"
print "<body>"
print "<h2>Hello %s %s %s</h2>" % (first_name, last_name, nick_name)
print "</body>"
print "</html>"


for x in range(1000):
    print '<h1>{}</h1>'.format(x)
