require 'data/models'

module UNE

  # csv header
  #-------------
  # Variable: Population - Aged 15 - 64
  # Units: Thousand People
  # Data Source: UN Population Division
  # Provider: UNEP GEO Data Portal (http://geodata.grid.unep.ch/)
  # Country Name,Sovereign,GEO Region,GEO Subregion,ISO 2 Code,ISO 3 Code,UN Code,Developed,GEO ID,2050
  #
  # At a minimum, we need: ISO 3 Code, GEO ID (actual value)
  #
  
  class BaseSchema < Sequel::Migration
    def up
      create_table! :nations do
        primary_key :id
        String :country_name, :null => false
        String :sovereign, :null => false
        String :geo_region
        String :geo_subregion
        String :iso2
        String :iso3, :null => false, :unique => true
        String :un
        Bool   :is_developed
        Integer :geo_id
        Integer :une_field_value # value of last fiels in CSV header
        
        index :iso3
      end
      
      create_table! :data_sets do
        primary_key :id
        
        String :variable_name, :null => false, :unique => true
        String :units
        String :data_source
        String :provider
        String :une_field_name  # name of last field in CSV header
        
        index :variable_name
      end
            
      create_table! :measurements do
        primary_key :id
        foreign_key :nation_id
        foreign_key :data_set_id
        
        Number :measurement
        
        index :measurement
        index :nation_id
        index :data_set_id
      end
    end
  
    def down
      drop_table :nations
      drop_table :data_sets
      drop_table :measurements
    end
  end

end