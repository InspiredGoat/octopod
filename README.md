# Contact Central? 
## (Taking suggestions for better name)

# NOT FINISHED USE AT YOUR OWN RISK

An encrypted database for contacts.
With scriptable hooks to social media and other services.

## Vision


## Planned Features


## Current Features


## Architecture


## Extending and plugins

## Config file

```

# start line with # for comment, empty lines are ignored

field:
    id: "fname"
    display_name: "First Name"
 
field:
    id: "lname"
    display_name: "First Name"
 
service:
    id: "phone"
    display_name: "Phone Number"
    hook_type: script

service:
    id: "twitter"
    display_name: "Twitter"
    dm_url: "https://twitter.com/{{cuid}}-{{uid}}"
    pub_url: "https://twitter.com/{{cuid}}"

service:
    id: "facebook"
    display_name: "Facebook"
    hook_type: script
    dm_url: "https://www.facebook.com/messages/t/{{cuid}}/"
    pub_url: "https://www.facebook.com/profile.php?id={{cuid}}"

service:
    id: "whatsapp"
    display_name: "Whatsapp"
    dm_url: "https://web.whatsapp.com"
    pub_url: "https://web.whatsapp.com"

```
