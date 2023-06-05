# 🐙 Octopod 🐙
## (Taking suggestions for better name)

# ⚠️⚠️ NOT FINISHED USE AT YOUR OWN RISK ⚠️⚠️
# ⚠️⚠️ MOST OF THE FOLLOWING SPEC IS SUBJECT TO CHANGE ⚠️⚠️

An encrypted database for contacts with scriptable hooks to social media and other services.

## Basic plan
3 projects with 3 scopes:
1. this project: a simple contact database library with tagging and hooks written in C
2. configurations: settings for interfacing database with proprietary services (so one per service)
3. the client(s): still undecided if I should make more than one client, but plan is have a very intuitive cross platform database client with all the bells and whistles (Comes with sensible configuration installed) ideally comes bundled as a single executable.

## Planned Features
- import contacts from social media (LinkedIn, Twitter, Gmail, etc.)
- social media hooks
    - click a button and dm a friend on their favourite platform
    - download a contact's feed
    - add custom functionality
- searching
- tags
- support for thousands of contacts
- database is fully encrypted

## Why?
Modern platforms use their high user counts to keep us trapped in their silos.
I think this is a technology problem.
Hear me out.

The fastest way to message your someone you know currently is:
1. remember where to message them
2. go to app/website
3. find contact
4. send messagk

The 1st and 2nd steps add friction to switching between platforms.
So slowly over time, one site is chosen per "type" of relationship.
For example one for work (LinkedIn), one for family (Facebook/Whatsapp), and one per friend group (Instagram/Discord/Snapchat).

In this environment competition is basically removed.
Things users might otherwise value (performance, design, privacy) become irrelevant.
Whichever platform lucks out becomes the first in its category and will stay there.

Here's where I come in.
This project (⚠️ ONCE FINISHED ⚠️) provides a fast and intuitive interface to get back control.
Want to message a friend? Here's how:
1. search friend in contact app
2. click message
3. friend's preferred chat opens with dm ready to go

If this process is faster than accessing a service normaly then people will not only be more open to new platforms, 
but they will also prioritize platforms that are highly integrated to this system.


In short, some of the benefits:
- Make communicating on the internet more deliberate
- Remove barrier to entry
- Keep up with

## Current Features
- [X] database create
- [X] database save
- [X] database load
- [X] database basic encryption (can't read fields as plain text)
- [X] database add tags
- [X] database add fields

## Architecture


## Extending and plugins

## Config file

Possible config file:

```

# start line with # for comment, empty lines are ignored

tags:
    immediate_family
    family
    close
    friend
    coworker
    client
    neighbour
    highschool
    university
    boss
    robot
    man
    woman

field:
    id: fname
    display_name: First Name
 
field:
    id: lname
    display_name: Last Name

field:
    id: lname
    display_name: Last Name
 
field:
    id: phone
    display_name: Phone Number
    hook_type: script
    dm_hook

field:
    id: twitter
    display_name: Twitter
    hook_type: url
    # In between {{ }} are custom variables which tell the server how to parse the url
    # "cuid" stands for "contact user id" it's the platform's configured "number" id
    # cname stands for 
    # cdata is for whatever value is stored at the field so email data, phone number data, or profile data.
    hook_dm: https://twitter.com/{{cuid}}-{{uid}}
    hook_public: https://twitter.com/{{cuid}}

field:
    id: facebook
    display_name: Facebook
    hook_type: script
    hook_dm: https://www.facebook.com/messages/t/{{cuid}}/
    hook_public: https://www.facebook.com/profile.php?id={{cuid}}

field:
    id: whatsapp
    display_name: Whatsapp
    dm_url: https://web.whatsapp.com
    # sadly this is as far as we can go with whatsapp
    pub_url: https://web.whatsapp.com

field:
    id: urbit
    display_name: Urbit
    # more advanced service logic can be imported through dll extensions
    hook_type: dll
    hook_file: urbit.dll
```



# DONE:
    - get tag names
    - set tags
    - add fields
    - set fields
    - create db
    - save db
    - basic encryption
    - load database
# TEST:
    - remove contact
    - request fields
    - add/remove tag names
    - scratch allocator works properly
# TODO:
    - request particular field
    - search by field
    - search by tag
    - queue fields???
# BUGS:
