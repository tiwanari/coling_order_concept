# puts str with vars (---)
def puts_line(str, length, out=STDOUT)
    word = str
    word = " " + str + " " if str.length != 0
    rest = length - word.length
    out_var = "-" * (rest / 2)
    output = out_var + word + out_var
    output += "-" if rest % 2 != 0
    out.puts output
end
