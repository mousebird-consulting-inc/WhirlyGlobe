#!/usr/bin/env ruby

require 'rubygems'
require 'mechanize'
require 'pp'

UNE_BASE_URL = "http://geodata.grid.unep.ch/"

a = Mechanize.new
a.get(UNE_BASE_URL) do |page|
  puts "requested #{UNE_BASE_URL}"
  
  # national search
  national_search = a.click( page.link_with(:id => /^National$/) )
  
  puts "Loading National search results"
  # submit search form
  search_results = national_search.form_with(:action => 'results.php') do |f|
  end.click_button
  
  # find the form with the most radio buttons
  puts "analyzing results form"
  
  File.open('results.html', 'w') { |f| f.write(search_results.body) }
  
  # the results page has a couple forms
  # figure out which out to use based on the number of radio buttons
  # We're interested in the form with a large number, indicating
  # several datasets available for download
  radio_counts = []
  search_results.forms.each do |form|
    radios = form.radiobuttons
    next unless radios
    
    radio_counts << radios.size
  end
  results_form = search_results.forms[radio_counts.index(radio_counts.max)]
  

  
  radio_buttons = results_form.radiobuttons_with(:name => /selectedID/)
  puts "radio button count: #{radio_buttons.size}"
  
  last_radio = nil  
  processed_ids = Hash.new
  
  radio_buttons.each do |radio|
    
    # skip dataset id's that we've already downloaded
    next if processed_ids.has_key?(radio.value)
    processed_ids[radio.value] = true
    
    # uncheck the radio button from the last download
    last_radio.uncheck if last_radio
    
    # select dataset
    radio.check
    last_radio = radio
    puts "selected radio button: #{radio.name} => #{radio.value}"

    # fill out hidden vars and submit
    results_form.selectedID = radio.value
    results_form.selectedDatasettype = 'National'
    results_form.layer = "int_boundary_lines"
    
    download_options = results_form.click_button
    
    # debug: save the resulting page
    File.open('download_options.html', 'w') { |f| f.write(download_options.body) }
    
    # find the csv link, if one exist
    csv_link = download_options.link_with(:text => /Comma Separated File/)
    unless csv_link
      # no csv - hit 'back' and keep processing
      a.back
      next
    end
    
    puts "csv_link: #{csv_link.inspect}"
    
    if ( csv_link )
      
      # fill out csv download form, and submit
      download_form = download_options.forms[0]
      download_form.type = 'csv'
      download_form.compute_completeness = 'on'
      download_form.action = 'http://geodata.grid.unep.ch/mod_download/download_file.php'

      puts "retrieving download page"
      download_page = a.submit(download_form)

      # debug: save download page
      File.open('download_page.html', 'w') { |f| f.write(download_page.body) }
      
      file_link = download_page.link_with(:href => /\.csv$/)
      
      unless file_link
        # no link? (this should not happen unless they don't generate the file)
        a.back; a.back
        next
      end
      
      # seriously?  The download URL has a ../temp in it.
      # normalize the 'path' segment of the URI, so that we can download it
      csv_uri = file_link.uri
      csv_uri.path = File.expand_path(csv_uri.path)
      
      puts "downloading csv from #{file_link.uri}"
      
      begin
        csv_contents = a.get(csv_uri)
        csv_filename = File.basename(csv_uri.path)
        File.open(File.join(File.dirname(__FILE__),"../import/une/#{csv_filename}"), 'w') { |f| f.write(csv_contents.body) }
      rescue => e
        # catch download errors - 
        # these are typically 401 auth errors
        #  I guess there is some data we can't access
        $stderr.puts "#{e.class}: #{e.message}"
      end
      a.back
      a.back
      
    end
  end
  
end