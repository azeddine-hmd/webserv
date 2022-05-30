# C++ Code Style

### type
* asterisk, reference and other type symbols should be after type definition (e.g "int* fd")
* const keyword should be after type definition and before type symbols (e.g "int const* fd)

### function
* use camelCase for function naming (e.g "addIntegerToTheListNow()")
* open curly bracket should be inline with function signature (e.g "type func() {")
* one space after open parentheses and before close parentheses  (e.g "int add( int x, int y )")
* don't join function body in one line (e.g "Server() {}" later is undesired behaviour)

### class
* Use upper case letters as word separators, lower case for the rest of the word in the class name.
  (e.g "class Calculator", "class ServerHandler", )
* resolving conflict would be by using namespace `ws` instead of prefixing with ft_ (e.g "ws::server()")
