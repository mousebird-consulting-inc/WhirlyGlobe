require 'sequel'

module UNE
  
  class Nation < Sequel::Model
    many_to_many  :data_sets, :left_key => :nation_id, :right_key => :data_set_id, :join_table => :measurements
  end
  
  class DataSet < Sequel::Model
    many_to_many  :nations, :left_key => :data_set_id, :right_key => :nation_id, :join_table => :measurements
  end
  
  class Measurement < Sequel::Model
    one_to_one   :nation
    one_to_one   :data_set
  end
  
end