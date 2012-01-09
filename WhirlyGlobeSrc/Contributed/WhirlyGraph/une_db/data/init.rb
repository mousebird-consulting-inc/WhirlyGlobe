require 'rubygems'
require 'sequel'
require 'yaml'

config_path = File.expand_path(File.join(File.dirname(__FILE__), "database.yml"))
content = File.new(config_path).read
settings = YAML::load(content)

db_path = File.expand_path(File.join(File.dirname(__FILE__), '..', settings['database']))
DB = Sequel.connect "#{settings['adapter']}://#{db_path}"

require 'data/models'