# Some FastCGI web services

When I wrote that critbit trie implementation some time back, I could not help notice that it would make a fast key-value store, and decided I would write one. The result of that silly idea is EnnoDB, possibly the worst NoSQL database in the world (pun only marginally intended, because that is actually my name).

It is recommended that you read the INSTALL.md file to get started. Contact me by email if you are interested in this project for any reason. If you use this in production, please a) let me know, and b) reconsider!

## counter

A simple web counter, my first shot at writing anything with FastCGI.

## prefix

Fast prefix search in a dictionary, for example as an auto-complete service to back a web form. See html/prefix.js for an example JavaScript implementation.

## ennodb

A minimalist key-value store that uses a journal to store its database. Every full-stack developer should at least write their own NoSQL store, right? See html/keyval.js for an example of how to use it from JavaScript. 
