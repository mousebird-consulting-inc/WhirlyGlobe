## WhirlyGlobe-Maply Documentation Fork

This fork is for the Jekyll conversion of WhirlyGlobe-Maply documentation. Here's how you'd set up to work on the docs.

### Ruby Versions and Gemsets

Perform a single-user installation of [RVM](http://rvm.io/rvm/install). Be sure that your installation passes the <a href="https://rvm.io/rvm/install#3-reload-shell-configuration-amp-test">```rvm
is a function``` test</a>.

Make sure you have installed or updated Bundler:

```
gem install bundler
gem update bundler
```

### Use Bundler to install Jekyll and Its Dependencies

```
$ cd WhirlyGlobe/
ruby-2.1.0 - #gemset created /Users/erictheise/.rvm/gems/ruby-2.1.0@WhirlyGlobe
ruby-2.1.0 - #generating WhirlyGlobe wrappers...........
$ bundle
Fetching gem metadata from https://rubygems.org/.......
Installing RedCloth (4.2.9)
...

Bundle complete! 1 Gemfile dependency, 85 gems now installed.
Use `bundle info [gemname]` to see where a bundled gem is installed.
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

### Addendum

If all that fails try: ```bundle exec jekyll build```

### Adding/Deleting Pages

The tutorial sidebar yields a linear navigation through the tutorial pages. The "prev/next" arrows will also rely on
this ordering. The ordering is controlled by ```_data/tutorial.yaml``` which is simply an ordered list of filenames in
the ```tutorials``` directory.
