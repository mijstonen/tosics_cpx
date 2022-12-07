////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CPX_STD_PHP_
#define CPX_STD_PHP_ 1

// PHP preprocessing effects line numbering because the C pre proc runs first
// Luckily their interaction allows to reset the source line so that line
// numbering (use of __LINE_ for messageses that point to source locations.
// __LINE__ queries the source line before PHP transformation,
// # line _LINE__+1 tells the compiler the line number following the line
// where this macro is used. Note that these macros should be used
// on a separate line. See rnr_source*.cpp You still can use < ? and ? > as long
// the number of lines is not affected by the PHP transformation (aka < ? $phpVariable ? > ).
//
// Macro's with PHP_ prefix are to be used inside PHP or associated with PHP.
//
#define CPP_MARKSRCLINE #include "marksourceline"
// include here uses preprocessed source i.s.o. true source for line determination.
//#define PHP_MARKSRCLINE echo "\n#include \"marksourceline\"\n";
#define PHP_MARKSRCLINE echo "\n# ",__LINE__+1,"  ",'"',__FILE__,'"',"\n";
#define  PHP_BEGIN <?php PHP_MARKSRCLINE
#define  PHP_END PHP_MARKSRCLINE ?>
#define PHP_VARDUMP_COMMENT(var) var_dump_comment($##var,#var)
// simple VARVAL variant to be used with primitive dubugging output to string
#define PHP_VARVAL(var) " $".#var."='". $##var ."' "
#define PHP_TIMESTAMP microtime(true)

#define PHP_ERROR_DIE(_message) {error_die(__FILE__,__LINE__,_message);}
#define PHP_HERE_BEGIN(_ID_) PHP_MARKSRCLINE echo <<< PHP_HERE_##_ID_
#define PHP_HERE_END(_ID_) PHP_HERE_##_ID_; PHP_MARKSRCLINE



//{______________________________________________________ cpx_stdphp.hpp _______________________________________________
 PHP_BEGIN
  // inside namespace Cpx; invoked by cpx  in shell script $CPX_HASH_COMPILE
  //____________________________________________________________________________________________________________________
#   define PHP_TRACE(_msgStr) trace(__LINE__,__METHOD__,_msgStr,false)

    class PhpPrepocessorWrapper
    // this is the root class for supporting globals,
    // its life time spans the entire PHP preprocessing phase
    {

        function __construct()
        {
            $this->traceHandle= fopen("php://stderr","a") /* STDERR */;

            echo $this->PHP_TRACE( "\n// PHP preprocessing started."
                                 . "\n// php version: ".phpversion()
                                 . " source: ".getenv("CPX_SOURCE_FILE")
                                 . " CPX_STACKLEVELCOUNT: ".getenv("CPX_STACKLEVELCOUNT")
                                 . " php preprocessor arguments: ".getenv("PRE_ARGS")
                                 . " work dir: " . workDir()
                                 );
            $php_ini_path = php_ini_loaded_file();
            global $argv;
            $this->PHP_TRACE(PHP_VARVAL(php_ini_path).PHP_EOL."\$argv = ".print_r($argv,true));
        }

        function __destruct()
        {
            global $PHP_started;
            $durationMicroSeconds = round((PHP_TIMESTAMP - $PHP_started)*1000000);
            echo $this->PHP_TRACE("\n// PHP spended $durationMicroSeconds micro-seconds, preprocessing ended \n");
            // trace was NOT set to STDERR. So...
            fclose($this->traceHandle);

            // The last thing that the php preprocessing does is...
            flush();
        }

        function trace(int $_line, string $_label, string $_message, bool $_lineNumbering=false) : string
        {
            $printedMessage=$_message;
            if ($_lineNumbering) {
                $messageLines=explode(PHP_EOL,$_message);
                $lineCount=0;
                $printedMessage="";
                foreach($messageLines as $messageLine) {
                    $printedMessage .= sprintf("\n    %03d: %s",++$lineCount,$messageLine);
                }
            }
            fwrite( $this->traceHandle, "$_line:$_label: $printedMessage\n");
            return $_message;
        }

    } // class PhpPrepocessorWrapper

    $TheGlobs=new PhpPrepocessorWrapper();
#   undef PHP_TRACE
  //____________________________________________________________________________________________________________________


    function php_Trace(int $_line, string $_label, string $_message, bool $_lineNumbering=false) : string
    {
        global $TheGlobs;
        return $TheGlobs->trace($_line,$_label,$_message,$_lineNumbering);
    }
# define PHP_TRACE(_msgStr) php_Trace(__LINE__,__FUNCTION__,_msgStr,false)
# define PHP_TRACE_LNRS(_msgStr) php_Trace(__LINE__,__FUNCTION__,_msgStr,true)


    function error_die(string $_file, int $_line, string $_errorMessage) : void
    // terminate execution but pass a #error to the compiler so
    // location and reason of the problem is reported
    {
        $fullMessage=sprintf("#error  In file: %s  at line: %d message: %s",$_file,$_line,$_errorMessage);
        PHP_TRACE(PHP_VARVAL(fullMessage));
        die($fullMessage);
    }

    function repeat(int $n)
    // usage: foreach(repeat(10) as $index) { $index = 0, 1, 2, ... 9 }
    {
        for ($i = 0; $i < $n; $i++) {
            yield $i;
        }
    }

    function CPPLineFileAndTokes(string $_ppline)
    // extract data from preprocessor lines (to be processed for other reporting)
    // note that the 1st and 2nd outputs are strings and the rest integers
    {
        $items = explode(' ', $_ppline);
        if ( $items[0]!='#' ) {
            PHP_ERROR_DIE("CPPLineFile parsing failed, expected ppline to start with '#'");
        }
        $GLOBALS["CPP_LINE"]=$items[1];
        $GLOBALS["CPP_FILE"]=$items[2];
        $tokens= array();
        foreach(array_slice($items,3) as $numTxt) $tokens[]= intval($numTxt);
        $GLOBALS["CPP_TOKENS"]= $tokens;
    }

    function getenv_or_die( string $envarName, $moreDieInfo="") : string
    // checked get environment variable
    {
        if ( ! ($evValue=getenv($envarName)) )  {
            PHP_ERROR_DIE("Failed to retrieve enviromment variable:'$envarName'. $moreDieInfo");
        }
        return $evValue;
    }

    function C_initializerListFrom(array $_itemValues, int $wrapAfter=100) : string
    // helper to convert arrays from php to C/++ eventually on multiple lines, avoiding unreadable long lines
    {
        $remainingLineColumns = $wrapAfter;
        $prior_ob_length=0;
        $itemsLeft=count($_itemValues);

        ob_start();
        echo '{';
        foreach( $_itemValues as $itemValue ){
            if (is_string($itemValue)) {
                echo '"'.$itemValue.'"';
            }
            else {
                echo $itemValue;
            }
            if (--$itemsLeft) {
                echo ',';
            }
            // insert line break when line is sufficiently used
            $now_ob_length = ob_get_length();
            $line_length_increment = $now_ob_length - $prior_ob_length;
            $remainingLineColumns -= $line_length_increment;
#if 0
            PHP_TRACE("varvals: ".
                    PHP_VARVAL(wrapAfter).PHP_VARVAL(prior_ob_length).
                    PHP_VARVAL(now_ob_length).PHP_VARVAL(line_length_increment).
                    PHP_VARVAL(remainingLineColumns).PHP_VARVAL(itemValue));
#endif
            $prior_ob_length = $now_ob_length;
#if 0
            PHP_TRACE(PHP_VARVAL(remainingLineColumns));
#endif
            if ( $remainingLineColumns<=0 ) {
                echo PHP_EOL;
                $remainingLineColumns = $wrapAfter;
            }
        }
        echo '}';
        return ob_get_clean();
    }

    function var_dump_comment($var,string $label="")
    // do a php var dump, that will be visible as a C/++ multiline comment of the php output (<file>.transfered)
    {
        echo PHP_EOL,"/* var_dump: ",$label,PHP_EOL;
         var_dump($var);
        echo PHP_EOL,"var_dump: ",$label,"*/",PHP_EOL;
    }

    function subProcess(string $_scripIdentifier, string $_embeddedScriptCommand): string
    {
        PHP_TRACE_LNRS(PHP_VARVAL(_embeddedScriptCommand));

        $results = shell_exec($_embeddedScriptCommand);
        if ( !$results ) {
            PHP_ERROR_DIE("$_scripIdentifier: Embedded script failed.");
        }
        //otherwhise PHP did not DIE
        return $results;
    }

    function phpInputSource() : string
    // to understand, try...
    //   php --no-php-ini --php-ini ../Kdevelop/cpx/etc/cpx_php.ini color.php SDCV Sdv sdv sdfv
    // The php script always appeares on argv[0]
    {
        global $argv;
        return $argv[0];
    }

    function workDir() : string
    { // could use __DIR__ however __*__ names potentionally could be C/++ preprocessor defined and earlier substituted
        return dirname(phpInputSource());
    }

    function tempnampost(string $directory, string $prefix, $postfix) : string
    // like tempnam() but with (postfix) file type extension
    {
        PHP_TRACE(PHP_VARVAL(directory).PHP_VARVAL(prefix).PHP_VARVAL(postfix));

        $from_createdfilename = tempnam( $directory, $prefix);
        $to_finalfilename  = $from_createdfilename . $postfix;
        $renamedOk = rename( $from_createdfilename, $to_finalfilename);

        if ( !$renamedOk ) {
            PHP_ERROR_DIE("Could not rename $from_createdfilename to $to_finalfilename.");
        }
        if ( !file_exists($to_finalfilename) ) {
            PHP_ERROR_DIE("Renaming failed, $to_finalfilename is asbent");
        }
        return $to_finalfilename;
    }

# define PHP_IN_BETWEEN_PATHS_CHAR ':'  // TBD: resolve linux/windows/mac platform dependency
# define PHP_CMDFTSTR_SCRIPT_TAG "%{<<<script}"


// TODO /fix need to enforce script file extension

function subProcessScript( string $_scripIdentifier, string $fileTypeExtension, string $_heredocScript,
                           string $_cmdFormatStr, mixed ...$_cmdVArgs
         ) : string

    // A more capable variant to handle a script of any embedded language. Before the script is executed,
    // it is saved to a temporary file, so that single/double quotes are allowed in the script as if it would be a
    // script file, but with all the benefits of CPP_C/++ and PHP preprocessing in the 'here' document
    {

        $tmpScriptFile =
            new class( tempnampost( workDir(), phpInputSource().".script.", ".".$fileTypeExtension))
            {
                    public /* php8 readonly*/ string
                $m_pathName; // remember path of temporary file

                    public function
                __construct(string $_tmpscriptpath) // save path for later deletion
                {
                    $this->m_pathName = $_tmpscriptpath;
                    PHP_TRACE("Saving embedded script to:'$this->m_pathName'");
                }
                    public function
                __destruct()
                // Whenever function subProcessScript completes/is-finally, the temorary path is deleted.
                {
                    PHP_TRACE("Removing temporary saved script: $this->m_pathName");
                    unlink( $this->m_pathName);
                }
                    public function
                store(string $_script) // Store script in a temporary path
                {
                    PHP_TRACE_LNRS("Saving ".PHP_VARVAL(_script));

                    $writecnt = file_put_contents( $this->m_pathName, $_script, LOCK_EX);

                    if ( !$writecnt )    PHP_ERROR_DIE("Could not file_put_contents($tmpscriptpath,...herdocScript...");
                    $scriptSize=strlen($_script);
                    if ( $writecnt!=$scriptSize )   PHP_ERROR_DIE("Incorrect file_put_contents results: ".
                                                                  PHP_VARVAL(writecnt)." != ".PHP_VARVAL(scriptSize));
                }
            } // class
        ; // tmpStoredScript

        $tmpScriptFile->store( $_heredocScript);
        $cmdFormatStrWithScriptPath=str_replace(PHP_CMDFTSTR_SCRIPT_TAG,$tmpScriptFile->m_pathName,$_cmdFormatStr);
        PHP_TRACE_LNRS(PHP_VARVAL(cmdFormatStrWithScriptPath));
        $command_str = sprintf($cmdFormatStrWithScriptPath, ...$_cmdVArgs);    // no need for escapeshellcmd
        PHP_TRACE_LNRS(PHP_VARVAL(command_str));

        // execute script
        $results = shell_exec( $command_str);
        if ( !$results )    PHP_ERROR_DIE("$_scripIdentifier: Embedded script failed.");

//otherwhise PHP did not DIE
        return $results;
    }

 PHP_END
//}______________________________________________________ cpx_stdphp.hpp _______________________________________________
#endif // CPX_STD_PHP_
