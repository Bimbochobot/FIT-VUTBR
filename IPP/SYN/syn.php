<?php 

#SYN:xsevci50

class Settings
{
    public $inFile = STDIN;
    public $formatFile = null;
    public $outFile = STDOUT;
    public $brLine = false;
}

# Used for printing errors to standard error output
# Param error: string describing the error.
function printError($error)
{
    fwrite(STDERR, "Error: " . $error . "\n");
}

# Used for storing information about one applied style
# before printing it out along with input.
class Tag
{
    public $pos = 0;
    public $format = null;
    public $open = true;
    
    # Comparing function for two tags based on their position.
    # Param a: first compared tag.
    # Param b: second compared tag.
    # Returns: -1 if lower, 0 if equal, 1 if greater
    public static function compare($a, $b)
    {
        if ($a->pos < $b->pos)
        {
            return -1;
        }
        elseif ($a->pos == $b->pos)
        {
            # Place close tags first
            if (!$a->open && $b->open)
            {
                return -1;
            }
            elseif ($a->open == $b->open)
            {
                return 0;
            }
            else 
            {
                return 1;
            }
        }
        else
        { 
            return 1;
        }
    }
    
    # Constructor for tag.
    # Param pos: position of tag.
    # Param format: format asociated with tag that will be printed.
    # Param open: True if tag is opening one, false otherwise.
    public function __construct($pos, $format, $open)
    {
        $this->pos = $pos;
        $this->format = $format;
        $this->open = $open;
    }
    
    # Represents tag as a string that can be printed.
    # Returns: string representing the tag.
    public function str()
    {
        if ($this->open)
            return $this->format->begin();
        else
            return $this->format->end();
    }
}

# Class for storing certain style defined in format file.
class Format
{
    # Constructor for format.
    # Param type: the type of tag, one of the defined enums in this class.
    # Param atr: additional attribute that will be stored with format.
    public function __construct($type, $atr = null)
    {
        $this->type = $type;
        $this->atr = $atr;
    }
    
    # Represents the format as an opening tag string.
    # Returns: format as opening tag string.
    public function begin()
    {
        switch ($this->type)
        {
        	case Format::BOLD:
        	    return "<b>";
        	case Format::ITALIC:
        	    return "<i>";
        	case Format::UNDERLINE:
        	    return "<u>";
        	case Format::TELETYPE:
        	    return "<tt>";
        	case Format::SIZE:
        	    return "<font size=" . $this->atr . ">";
        	case Format::COLOR:
        	    return "<font color=#" . $this->atr . ">";
        	case Format::BRLINE:
        	    return "<br />";
        }
    }
    
    # Represents the format as a closing tag string.
    # Returns: format as closing tag string.
    public function end()
    {
        switch ($this->type)
        {
        	case Format::BOLD:
        	    return "</b>";
        	case Format::ITALIC:
        	    return "</i>";
        	case Format::UNDERLINE:
        	    return "</u>";
        	case Format::TELETYPE:
        	    return "</tt>";
        	case Format::SIZE:
        	case Format::COLOR:
        	    return "</font>";
        	case Format::BRLINE:
        	    return "";
        }
    }
    
    # Enums used for specifying the supported formats.
    const BOLD = 0;
    const ITALIC = 1;
    const UNDERLINE = 2;
    const TELETYPE = 3;
    const SIZE = 4;
    const COLOR = 5;
    const BRLINE = 6;
    
    private $type;
    private $atr;
}

# Class that holds information about single rule - the regex it uses
# to match strings and array of formats that will be aplied.
class Rule
{
    public $regex = "";
    public $formats = array();
}

# Class that does the formatting operations.
class Formater
{
    private $input = "";
    private $rules = array();
    private $tags = array();
    
    # Initializes the instance into usable state.
    # Most notably reads the input and format files and stores that information.
    # Param settings: settings class
    public function initialize($settings)
    {
        # Read whole input
        while (!feof($settings->inFile))
        {
            $buffer = fread($settings->inFile, 100);
            if ($buffer === false)
                return 1;
            else
                $this->input .= $buffer;
        }
        
        # Read format file line by line and save records as rules
        while ($settings->formatFile != null && !feof($settings->formatFile))
        {
            $rule = new Rule();
            $line = rtrim(fgets($settings->formatFile));
            if (strlen($line) == 0)
                continue;
            $parts = preg_split("/\t+/u", $line, 2);

            if (count($parts) != 2)
            {
                printError("Wrong format file format (tabs): " . $line . "\n");
                return 4;
            }
            
            $regex = $this->convertRegex($parts[0]);
            if ($regex === null)
            {
                printError("Wrong format file format (regex): " . $line . "\n");
                return 4;
            }
            
            $formats = $this->parseFormats($parts[1]);
            if ($formats === null)
            {
                printError("Wrong format file format (commands): " . $line . "\n");
                return 4;
            }
            
            $rule->regex = $regex;
            $rule->formats = $formats;
            array_push($this->rules, $rule);
        }
        
        if ($settings->brLine)
        {
            $rule = new Rule();
            $rule->regex = $this->convertRegex("%n");
            $rule->formats = array(new Format(Format::BRLINE));
            array_push($this->rules, $rule);
        }
        
        return 0;
    }
    
    # Formats the input using loaded rules and stores that information.
    public function format()
    {
        $this->tags = array();
        
        foreach ($this->rules as $rule)
        {
            preg_match_all($rule->regex, $this->input, $matches, PREG_OFFSET_CAPTURE | PREG_SET_ORDER);
            
            foreach ($matches as $match)
            {
                if (count($match) == 0 || strlen($match[0][0]) == 0)
                    continue;
                
                $end = $match[0][1] + strlen($match[0][0]);
                foreach ($rule->formats as $format)
                {
                    array_push($this->tags, new Tag($match[0][1], $format, true));
                    array_push($this->tags, new Tag($end, $format, false));
                }
            }
        }
        $this->tags = mergeSort($this->tags, "Tag::compare");
    }
    
    # Writes current state of formatter as formated text to the output.
    # Param settings: settings class
    public function write($settings)
    {
        $tagStack = array();
        $b = 0;
        $e = 0;
        
        foreach ($this->tags as $tag)
        {
            $e = $tag->pos;
            
            # Write pending closing tags from stack
            if ($tag->open || $e > $b)
            {
                while (count($tagStack) > 0)
                {
                    fwrite($settings->outFile, array_pop($tagStack)->str());
                }
            }
            
            # Write part of inptu file between tags
            if ($e > $b)
            {
                fwrite($settings->outFile, substr($this->input, $b, $e - $b));
            }
            
            # If opening tag, write it instantly, otherwise put it on stack
            if ($tag->open)     
            {           
                fwrite($settings->outFile, $tag->str());
            }
            else
            {
                array_push($tagStack, $tag);
            }
            
            $b = $e;
        }
        
        # Finish writing of all closing tags
        while (count($tagStack) > 0)
        {
            fwrite($settings->outFile, array_pop($tagStack)->str());
        }
        
        $e = strlen($this->input);
        if ($e > $b)
        {
            fwrite($settings->outFile, substr($this->input, $b, $e - $b));
        }
    }
    
    # Function for negating the PHP regex.
    # Param regex: original regex to be negated.
    # Returns: negated regex.
    private function negateRegex($regex)
    {
        if (strlen($regex) > 0)
        {
            if ($regex[0] == "[")
            {
                # Just add negation mark into character group
                $regex = "[^" . substr($regex, 1);
            }
            else
            {
                # Wrap in character group and negate
                $regex = "[^" . $regex . "]";
            }
        }
        
        return $regex;
    }
    
    # Converts regex from IFJ representation into PHP one.
    # Param ifjRegex: regex written in IFJ representation.
    # Returns: PHP regex that matches the same strings.
    private function convertRegex($ifjRegex)
    {
        $regex = "";
        $regexPart = "";
        $escape = false;
        $special = false;
        $quantifier = false;
        $lbrace = false;
        $negate = false;
        $openPars = 0;
        $first = true;
        
        for ($i = 0; $i < strlen($ifjRegex); $i++)
        {
            if ($escape)
            {
                switch($ifjRegex[$i])
                {
                    case "s":
                        $regexPart = "[ \t\n\r\f\v]";
                        break;
                    case "a":
                        $regexPart = "[\D\d]";
                        break;
                    case "d":
                        $regexPart = "\d";
                        break;
                    case "l":
                        $regexPart = "[a-z]";
                        break;
                    case "L":
                        $regexPart = "[A-Z]";
                        break;
                    case "w":
                        $regexPart = "[a-zA-Z]";
                        break;
                    case "W":
                        $regexPart = "[a-zA-Z\d]";
                        break;
                    case "t":
                        $regexPart = "\t";
                        break;
                    case "n":
                        $regexPart = "\n";
                        break;
                    # Escape special characters
                    case ".":
                    case "|":
                    case "!":
                    case "*":
                    case "+":
                    case "(":
                    case ")":
                    case "%":
                        $regexPart = "\\" . $ifjRegex[$i];
                        break;
                    default:
                        return null;
                }
                $escape = false;
            }
            else 
            {
                switch($ifjRegex[$i])
                {
                    case ".":
                        if ($special || $first)
                            return null;
                        $special = true;
                        break;
                    case "|":
                        if ($special || $first)
                            return null;
                        $regex .= $ifjRegex[$i];
                        $special = true;
                        break;
                    case "!":
                        if ($special)
                            return null;
                        $special = true;
                        $negate = true;
                        break;
                    case "*":
                        if ($special || $quantifier || $first)
                            return null;
                        $regex .= $ifjRegex[$i];
                        $quantifier = true;
                        break;
                    case "+":
                        if ($special || $quantifier || $first)
                            return null;
                        $regex .= $ifjRegex[$i];
                        $quantifier = true;
                        break;
                    case "(":
                        if ($negate)
                            return null;
                        $openPars++;
                        $lbrace = true;
                        $regex .= $ifjRegex[$i];
                        break;
                    case ")":
                        if ($negate || $lbrace || $first)
                            return null;
                        $openPars--;
                        if ($openPars < 0)
                            return null;
                        $regex .= $ifjRegex[$i];
                        break;
                    case "%":
                        $escape = true;
                        # Can be followed only by character or group.
                        $special = $quantifier = $lbrace = false;
                        break;
                    # Escape PHP regex characters.
                    case "[":
                    case "]":
                    case "{":
                    case "}":
                    case "\\":
                    case "^":
                    case "$":
                    case "?":
                    case "/":
                        $regexPart = "\\";
                    default:
                        if (ord($ifjRegex[$i]) < 32)
                        {
                            return null;
                        }
                        $regexPart .= $ifjRegex[$i];
                        $special = $quantifier = $lbrace = false;
                        break;
                }
            }
            
            if ($regexPart !== "")
            {
                if ($negate)
                {
                    $regex .= $this->negateRegex($regexPart);
                    $negate = false;
                }
                else
                    $regex .= $regexPart;

                $regexPart = "";
            }
            
            $first = false;
        }
        
        # Regex ended unfinished
        if ($openPars != 0 || $negate || $special || $escape)
        {
            return null;
        }
        
        # Make regex multiline
        return "/" . $regex . "/um";
    }
    
    # Parses formats from format file into internal representation.
    # Param formats: string with formats to parse.
    # Returns: array with parsed formats.
    private function parseFormats($formats)
    {
        $splitFormats = preg_split("/,[ \t]*/u", $formats);
        $parsedFormats = array();
        foreach ($splitFormats as $format)
        {
            switch ($format)
            {
            	case "bold":
            	    array_push($parsedFormats, new Format(Format::BOLD));
            	    break;
            	case "italic":
            	    array_push($parsedFormats, new Format(Format::ITALIC));
            	    break;
            	case "underline":
            	    array_push($parsedFormats, new Format(Format::UNDERLINE));
            	    break;
            	case "teletype":
            	    array_push($parsedFormats, new Format(Format::TELETYPE));
            	    break;
            	default:
            	    if (substr($format, 0, 5) === "size:")
            	    {
            	        $size = substr($format, 5);
            	        if ($size < 1 || $size > 7)
            	            return null;
            	        
            	        array_push($parsedFormats, new Format(Format::SIZE, $size));         	        
            	    }
            	    elseif (substr($format, 0, 6) === "color:")
            	    {
            	        $color = substr($format, 6);
            	        if (preg_match("/[0-9A-F]{6}/u", $color) !== 1)
            	            return null;
            	        
            	        array_push($parsedFormats, new Format(Format::COLOR, $color));
            	    }
            	    else 
            	    {
            	        return null;
            	    }
            	    break;
            }
        }  

        return $parsedFormats;
    }
}

# Mergesort implementation.
# Param array: array to sort.
# Param comparator: function to use as a comparator.
# Returns: sorted array.
function mergeSort($array, $comparator)
{
    if (count($array) <= 1)
        return $array;
    
    $middle = count($array) / 2;
    $left = mergeSort(array_slice($array, 0, $middle), $comparator);
    $right = mergeSort(array_slice($array, $middle), $comparator);
    return merge($left, $right, $comparator);
}

# Merge operation for mergesort.
# Param left: array on the left from pivot to merge.
# Param right: array on the right from pivot to merge.
# Param comparator: function used as a comparator during merging.
# Returns: merged arrays.
function merge($left, $right, $comparator)
{
    $result = array();
    $i = 0; $cl = count($left);
    $j = 0; $cr = count($right);
    
    while ($i < $cl && $j < $cr)
    {
        if (call_user_func($comparator, $left[$i], $right[$j]) <= 0)
        {
            array_push($result, $left[$i]);
            $i++;
        }
        else 
        {
            array_push($result, $right[$j]);
            $j++;
        }
    }
    
    if ($i < $cl)
    {
        $result = array_merge($result, array_splice($left, $i));
    }
    elseif ($j < $cr)
    {
        $result = array_merge($result, array_splice($right, $j));
    }
    
    return $result;
}

# Parses arguments into internal representation - settings class
# Param settings: settings class to be filled according to command line arguments
function parseArguments(&$settings)
{
    global $argv;
    $inSet = false;
    $formatSet = false;
    $outSet = false;
    $brSet = false;
    
    # Ignore script filename
    $args = array_slice($argv, 1);
    
    foreach ($args as $arg)
    {
        $e = explode("=",$arg);
        if (count($e) == 2)
        {
            switch ($e[0])
            {
                case "--input":
                    if (!$inSet)
                    {
                        if (!file_exists($e[1]))
                        {
                            printError("Input file doesn't exist!");
                            return 2;
                        }
                        $inFile = fopen($e[1], "r");
                        if ($inFile === false)
                        {
                            printError("Input file couldn't be opened!");
                            return 2;
                        }
                        $inSet = true;
                        $settings->inFile = $inFile;
                    }
                    else
                    {
                        printError("Input parameter used more than once!");
                        return 1;
                    }
                    break;
                    
                case "--format":
                    if (!$formatSet)
                    {
                        if (file_exists($e[1]))
                        {
                            $formatFile = fopen($e[1], "r");
                            if ($formatFile === false)
                            {
                                $formatFile = null;
                            }
                        }
                        else 
                        {
                            $formatFile = null;
                        }
                            
                        $formatSet = true;
                        $settings->formatFile = $formatFile;
                    }
                    else
                    {
                        printError("Format parameter used more than once!");
                        return 1;
                    }
                    break;
                    
                case "--output":
                    if (!$outSet)
                    {
                        $outFile = fopen($e[1], "w");
                        if ($outFile === false)
                        {
                            printError("Output file couldn't be opened for write!");
                            return 3;
                        }
                        $outSet = true;
                        $settings->outFile = $outFile;
                    }
                    else
                    {
                        printError("Output parameter used more than once!");
                        return 1;
                    }
                    break;
                    
                default:
                    printError("Unknown argument used!");
                    return 1;
            }
        }
        else if (count($e) == 1)
        {
            switch ($e[0])
            {
                case "--help":
                    if (count($args) == 1)
                    {
                        $help = "First project for IPP course: Syntactic highlighter\n";
                        $help .= "Applies HTML style format tags specified in format file onto input stream.\n";
                        $help .= "For extensive information about rules and formats see specification: \n";
                        $help .= "https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IPP-IT/projects/2013-2014/Zadani/syn.pdf\n\n"; 
                        $help .= "Options:\n";
                        $help .= "  --help                Outputs this text, can't be combined with other options.\n";
                        $help .= "  --input=<inputFile>   Takes input stream from <inputFile> instead\n"; 
                        $help .= "                        of standard input.\n";
                        $help .= "  --format=<formatFile> Formats input with rules specified in <formatFile>.\n";
                        $help .= "                        If empty, stream is copied without change.\n";
                        $help .= "  --output=<outputFile> Writes formated stream into <outputFile>\n";
                        $help .= "                        instead of standard output.\n";
                        $help .= "  --br                  Every line of output will be terminated with\n";
                        $help .= "                        additional <br /> tag.\n";
                        
                        fwrite($settings->outFile, $help);

                        return -1;  
                    }
                    else 
                    {
                        printError("Help parameter not used alone!");
                        return 1;
                    }
                    break;
                    
                case "--br":
                    if ($brSet)
                    {
                        printError("BR parameter used more than once!");
                        return 1;
                    }
                    $brSet = true;
                    $settings->brLine = true;
                    break;
                    
                default:
                    printError("Unknown argument used!");
                    return 1;
            }
        }
        else 
        {
            printError("Unknown argument used!");
            return 1;
        }
    }
    
    return 0;
}

$settings = new Settings();
$result = parseArguments($settings);

if ($result > 0)
{
    exit ($result);
}
elseif ($result < 0)
{
    exit(0);
}

$formater = new Formater();
$result = $formater->initialize($settings);
if ($result > 0)
{
    exit($result);
}

$formater->format();
$result = $formater->write($settings);
exit($result);

?>