## WhirlyGlobe-Maply Documentation Fork

This fork is for the Jekyll conversion of WhirlyGlobe-Maply documentation. Here's how you'd set up to work on the docs.

### Ruby Versions and Gemsets

Perform a single-user installation of [RVM](http://rvm.io/rvm/install). Be sure that your installation passes the ```rvm
is a function``` test.

### Use Bundler to install Jekyll and Its Dependencies

```
$ cd WhirlyGlobe/
ruby-2.1.0 - #gemset created /Users/erictheise/.rvm/gems/ruby-2.1.0@WhirlyGlobe
ruby-2.1.0 - #generating WhirlyGlobe wrappers...........
$ bundle
Fetching gem metadata from https://rubygems.org/.......
Installing RedCloth (4.2.9)
Installing i18n (0.6.11)
Using json (1.8.1)
Installing minitest (5.4.2)
Installing thread_safe (0.3.4)
Installing tzinfo (1.2.2)
Installing activesupport (4.1.6)

...

Installing html-pipeline (1.9.0)
Installing jekyll-mentions (0.1.3)
Installing jekyll-redirect-from (0.6.2)
Installing jekyll-sitemap (0.6.0)
Installing jemoji (0.3.0)
Installing maruku (0.7.0)
Installing rdiscount (2.1.7)
Installing github-pages (28)
Using bundler (1.5.3)
Your bundle is complete!
Use `bundle show [gemname]` to see where a bundled gem is installed.
Post-install message from html-pipeline:
-------------------------------------------------
Thank you for installing html-pipeline!
You must bundle Filter gem dependencies.
See html-pipeline README.md for more details.
https://github.com/jch/html-pipeline#dependencies
$
```

### Run Jekyll to Build Pages and Watch for Edits

```
$ jekyll serve --baseurl ''
Configuration file: /Users/erictheise/Projects/erictheise/WhirlyGlobe/_config.yml
            Source: /Users/erictheise/Projects/erictheise/WhirlyGlobe
       Destination: /Users/erictheise/Projects/erictheise/WhirlyGlobe/_site
      Generating...
                    done.
 Auto-regeneration: enabled for '/Users/erictheise/Projects/erictheise/WhirlyGlobe'
Configuration file: /Users/erictheise/Projects/erictheise/WhirlyGlobe/_config.yml
    Server address: http://0.0.0.0:4000/
  Server running... press ctrl-c to stop.
```

This'll start a [local server running on port 4000](http://localhost:4000/tutorial/getting_started.html).

### Adding/Deleting Pages

The tutorial sidebar yields a linear navigation through the tutorial pages. The "prev/next" arrows will also rely on
this ordering. The ordering is controlled by ```_data/tutorial.yaml``` which is simply an ordered list of filenames in
the ```tutorials``` directory.
