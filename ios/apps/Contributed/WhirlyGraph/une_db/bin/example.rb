require 'une'
require 'pp'

# list dataset contents
pp UNE::DataSet.filter.sql
pp UNE::DataSet.all

# list dataset names
q = UNE::DataSet.select(:variable_name)
pp q, q.all

