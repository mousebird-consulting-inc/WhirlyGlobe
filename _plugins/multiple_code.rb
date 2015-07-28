module Jekyll

  class MultipleCode < Liquid::Block

    def initialize(tag_name, text, tokens)
      super
      @id = text
    end

    def render(context)

      @output = "";
      links = "";
      content = "";

      fragments = super.split('{----}');

      fragments.each_with_index do |fragment, index|

        links += '<a href="#" data-index="' + index.to_s + '">&nbsp;</a>'

        content += '<li data-index="' + index.to_s + '">'
        content += fragment
        content += "</li>"
      end

      @output += "<div class=\"multiple_code_header\">" + links + "</div>"
      @output += "<ul class=\"multiple_code\">"
      @output += content
      @output += "</ul>"

      "#{@output}"
    end

  end

end

Liquid::Template.register_tag('multiple_code', Jekyll::MultipleCode)
