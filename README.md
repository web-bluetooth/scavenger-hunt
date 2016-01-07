# Beacon Scavenger Hunt

This code demonstrates one possible use of BLE URIBeacons. In this demo multiple beacons are loaded with the same piece of code. Each beacon will add its mac address to the query string to be sent to the server. This allows each board to be paired wtih unique information in the server database. In this instance a company ID and passphrase are associated with each beacon. This information is then displayed either in the notification tray on the phone or on the webpage when connected to.

## Using it

To use the app please make sure the shortened URL in the mbed code points to a server running the server code (in this case we are providing Google App Engine codeas an example). When a board is first connected to the server the user will need assign it a company ID and a passphrase. On later connections to the server this information will be returned to the requesting device (usually a smartphone / tablet)

In short there are two phases of operation, setup and use.

### Setup:

1. make sure shortened url in mbed code points to app engine server running code
1. load code to mbed enabled BLE board
1. load app engine with provided code
1. connect for first time to server via physical web app and register the device

### Use:

1. scan with physical web app
1. uribeacon is found, companyID and passphrase are displayed in notification bar (alternatively can connect to webpage for same info)

## Examples

This photo shows information from the server being shown in the notification drawer on android. There are 2 beacons shown, the first beacon is in phase 2: Use, it has been initialized on the server with a company name of 'Random company' and a passphrase of 'Wombats are awesome!!'. The other beacon is in phase 1: setup, and has not been initialized, so it displays the "please configure me" message.

![SUCH IMAGE MUCH WOW](https://developer.mbed.org/media/uploads/mbedAustin/screenshot_2015-04-16-21-10-16-1-.png)

## Constraints

This example relies on passing along a query string '?q=" from the shortened url to the long url. Not all shortening services do this. We recommend using tiny.cc or snurl.com as they will pass along the '?q=.....' to the server. Currently the shortened URL in the example points to mbeduribeacon.appspot.com . The duration of the shortened URL is not guaranteed, so it may revert back to nothing in the future. Also the appspot server may go down in the future, thus we have provided the source code for both sides to the user.

This example also relies on the Physical Web app (available for iOS and Android). As of the date of this writing it works just fine, but since we at mbed do not control this app, this may change in the future.

Sometimes something will take a little time to buffer things, im not sure if its the google app engine or the proxy that URIBeacons are run through on the PhyscialWeb app. This means there may be up to a 20min delay between when you do the initial setup of the device and when it starts showing up correctly on your devices notification tray, but if you click through to the webpage it should work immediately after being setup.

## Technical Details

This example works by forwarding the mac address of the device in the query string '?q=MACADDR' to the server. The server will return a webpage based on the mac address in the query string. Not all url shortening services do this, so we used snurl.com, because it does. The PhysicalWeb phone app will scan the beacon, ping the server, and display the MetaData (description, favicon, title, url) fields in the notification drawer and in the physical web app. This entire demo relies on the PhysicalWeb app to handle the phone side interaction.

## Summary

This is a cool demo of how to use URIBeacons to facilitate connecting the physical and digital worlds. It is by no means an end all be all, but is just one way to use them. Go build something awesome!

## Other things

This is the the google app engine code necessary to run this example. If you would like to run your own server you can sign up for a free app engine account and use this python code as your example code.

```python
from google.appengine.api import users
import cgi
 
import webapp2
 
import pprint
 
register = {
    'test':{'companyid':'Awesomeness Inc.','passphrase':'wombats of the doom'},
}
 
ADD_PAGE_HTML = '''\
<html>
    <head>
        <title>mbed Powered Scavenger Hunt</title>
        <meta name="description" content="This device is not recognized. Please click to add device to database.">
    </head>
  <body>
    <h2>This device is not recognized by the web server, please register it.</h2>
    <form action="/add" method="post">
      <div>Board:<textarea name="boardid">DEFAULTKEY</textarea></div>
      <div>CompanyID:<textarea name="companyid" rows="1" cols="60"></textarea></div>
      <div>passphrase<textarea name="passphrase" rows="1" cols="60"></textarea></div>
      <div><input type="submit" value="submit"></div>
    </form>
  </body>
</html>
'''
 
RESPONSE_PAGE_HTML = '''\
<html>
    <head>
        <title>mbed Powered Scavenger Hunt</title>
        <meta name="description" content="DEFAULTCOMPANY's password is 'DEFAULTVALUE'">
    </head>
    <body>
        <b>DEFAULTCOMPANY</b>'s password is '<i>DEFAULTVALUE</i>'
 
    </body>
 
</html>
'''
 
# main page checks dictionary 
#   if key exists display value
#   if key doesnt exist add it to dictionary
class MainPage(webapp2.RequestHandler):
    def get(self):
        x = self.request.get('q').lower()
        if x in register.keys():
            self.response.write(RESPONSE_PAGE_HTML.replace('DEFAULTVALUE',register[x]['passphrase']).replace('DEFAULTCOMPANY',register[x]['companyid']))
        else:
            self.response.write(ADD_PAGE_HTML.replace('DEFAULTKEY',x))
 
 
# the debug page prints out all key value pairs
class DebugPage(webapp2.RequestHandler):
    def get(self):
        pp = pprint.pformat(register,indent=6)
        self.response.write(pp)
 
# the add page takes a post message and adds the keys and values to the dictionary
class AddPage(webapp2.RequestHandler):
    def post(self):
        boardid     = cgi.escape(self.request.get('boardid'))
        companyid   = cgi.escape(self.request.get('companyid'))
        passphrase  = cgi.escape(self.request.get('passphrase'))
        register[boardid] = {'passphrase':passphrase,'companyid':companyid}
        self.response.write("Added company:'<b>"+companyid+"</b>' with passphrase:'<i>"+passphrase+"</i>'")
 
application = webapp2.WSGIApplication(
    [
        ('/', MainPage),
        ('/debug',DebugPage),
        ('/add',AddPage),
], debug=True)
```
