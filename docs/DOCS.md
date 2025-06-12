# GROUPS

Acces control design

Multiple 'domains' need acces to the data

Examples of domains
* administrator_user
* unauthenticated_user
* bluetooth_stack
* internal

On a parameter level the domains need to be set which are allowed to acces this parameter
This is done by using a bitmap

It also makes distinctions between the type of access

Types of access:
* read
* write
* log
* lock/unlock (future extension)
* reset (future extension)



# ID

Every parameter has an unique identifier, this is used to identify the parameter in the system
The global id is determined by a waterfall system, based on the shifting of the parent groups


