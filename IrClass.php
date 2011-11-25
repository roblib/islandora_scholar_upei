<?php


/*
 * Created on 28-Oct-08
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
class IrClass {

    function IrClass() {
        drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    }

    function createHomePage() {
        $output = '<Strong>Quick Search</strong><br /><table class="table-form"><tr>' . drupal_get_form('scholar_quick_search_form') . '</tr></table>';
        $output .= '<div>or</div><br />';
        $output .= '<strong>Browse by name</strong><br /><table class="table-form"><tr>' . drupal_get_form('scholar_browse_by_user_form') . '</tr></table>';
        $output .= '<h3>Browse by Department</h3>';
        $output .= $this->browse_by_department();
        return $output;
    }

    function quick_search_form_submit($form_values) {
        $type_id = $form_values['type'];

        $searchString = $form_values['fedora_terms'];
        $orderBy = $form_values['order_by'];

        drupal_goto("fedorair/custom_search/$type_id/$searchString/1/$orderBy");
    }
    //not in use yet
    function advanced_search_form($results = null) {

        $types = array (
            'refworks.t1' => 'Title',
            'refworks.a1' => 'Author',
            'dsm.text' => 'Text',
            'refworks.jf' => 'Journal Title'
        );
        $andOrArray = array (
            'AND' => 'and',
            'OR' => 'or'
        );
        $form = array ();

        $form['search_type']['type1'] = array (

            '#title' => t(''
            ), '#type' => 'select', '#options' => $types);
        $form['fedora_terms1'] = array (

            '#type' => 'textfield',
            '#title' => t(''
            ), '#required' => true);
        $form['andor1'] = array (
            '#title' => t(''
            ), '#type' => 'select', '#default_value' => 'AND', '#options' => $andOrArray);
        $form['search_type']['type2'] = array (

            '#title' => t(''
            ), '#type' => 'select', '#options' => $types);
        $form['fedora_terms2'] = array (

            '#type' => 'textfield',
            '#title' => t(''
            ));
        $form['andor2'] = array (
            '#title' => t(''
            ), '#type' => 'select', '#default_value' => 'AND', '#options' => $andOrArray);
        $form['search_type']['type3'] = array (

            '#title' => t(''
            ), '#type' => 'select', '#options' => $types);
        $form['fedora_terms3'] = array (

            '#type' => 'textfield',
            '#title' => t(''
            ),);

        $form['submit'] = array (
            '#type' => 'submit',
            '#value' => t('search'
            ));
        return $form;

    }
    //not used yet
    function theme_advanced_search_form($form) {
        $output = '<table><tr><td>&nbsp;</td><td>';
        $output .= drupal_render($form['search_type']['type1']) . '</td><td>';
        $output .= drupal_render($form['fedora_terms1']) . '</td></tr>';
        $output .= '<tr><td>' . drupal_render($form['andor1']) . '</td><td>' . drupal_render($form['search_type']['type2']) . '</td><td>';
        $output .= drupal_render($form['fedora_terms2']) . '</td></tr>';
        $output .= '<tr><td>' . drupal_render($form['andor2']) . '</td><td>' . drupal_render($form['search_type']['type3']) . '</td><td>';
        $output .= drupal_render($form['fedora_terms3']) . '</td></tr>';
        $output .= '<tr><td colspan="3">' . drupal_render($form['submit']) . '</td></tr></table>';
        $output .= drupal_render($form);
        return $output;
    }

    function quick_search_form() {
        $orderBy = array (
            '0' => 'Order By',
            '1' => 'Date',
            '2' => 'Relevance'
        );
        $types = array (
            'refworks.t1' => 'Title',
            'refworks.a1' => 'Author',
            'dsm.text' => 'Text',
            'refworks.jf' => 'Journal Title'
        );
        $form = array ();

        $form['search_type']['type'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#title' => t(''
            ), '#type' => 'select', '#weight' => -8, '#options' => $types);
        $form['fedora_terms'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#type' => 'textfield',
            '#title' => t(''
            ), '#required' => true, '#weight' => 3);
        $form['fedora_rank']['order_by'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#type' => 'select',
            '#options' => $orderBy,
            '#title' => t(''
            ), '#required' => true, '#weight' => 4);

        $form['submit'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#type' => 'submit',
            '#value' => t('search'
            ), '#weight' => 10);
        return $form;
    }
    //gets fullname when passed a username
    //we are currently using a file to get this info
    //originally we sucked this out of the db but since most faculty have
    //never logged in we never get their info out of ldap and since there is no
    //easy way to determine if a users is faculty or staff in our ldap we are a text file list
    //to generate the dropdowns
    function get_user_by_username($username) {
        module_load_include('inc','Fedora_Repository', 'api/fedora_utils');
        if (!isset ($username)) {
            drupal_set_message(t('No username provided!'), 'error');
        }
        $result = db_query("SELECT users.uid FROM {users} where users.name = '%s'", $username);
        $user_id = db_fetch_object($result);
        $user = user_load(array (
            'uid' => $user_id->uid
            ));
        return $user->profile_name;//.' ('.$user->name.') ';

    }
    //queries the object for its refworks datastream so we can transform it
    //to html
    function show_full_record($pid) {
        global $user;

        module_load_include('php', 'Fedora_Repository', 'ObjectHelper');

        $objectHelper = new ObjectHelper();
        $refworksRecord = $objectHelper->getStream($pid, 'refworks', 1);
        $path = drupal_get_path('module', 'scholar');
        $proc = null;
        if (!$refworksRecord) {
            drupal_set_message(t('No Results for refworks record!'));
            return ' '; //no results
        }

        try {
            $proc = new XsltProcessor();
        } catch (Exception $e) {
            drupal_set_message(t('Error loading results xslt! ') . $e->getMessage());
            return ' ';
        }

        //inject into xsl stylesheet
        $proc->setParameter('', 'pid', $pid); //needed in our xsl
        $proc->setParameter('', 'objectsPage', base_path());
        $xsl = new DomDocument();

        $test = $xsl->load($path . '/xsl/refworks.xsl');
        if (!isset ($test)) {
            drupal_set_message(t('Error loading refworks xslt!'));
            return t('Error loading search results xslt! ');
        }

        $input = new DomDocument();
        $didLoadOk = $input->loadXML($refworksRecord);
        $status = $input->getElementsByTagName('status');
        $status = $status->item(0)->nodeValue;

        if (!isset ($didLoadOk)) {
            drupal_set_message(t('Error loading search results!'));
            return t('Error loading search results! ');

        } else {

            $proc->importStylesheet($xsl);

            $newdom = $proc->transformToDoc($input);

            $output .= $newdom->saveXML();

        }
        //we should change this to only show the form if the user can actually edit it
        //we know the user and we have the refworks record so should be doable
        if (0 < $user->uid && ('NO_OBJ' == $status||$status==null)) { //show the author stuff
            $status="test";
            $output .= '<br />';
            module_load_include('php', 'Fedora_Repository', 'plugins/ShowStreamsInFieldSets');
            $romeo = new ShowStreamsInFieldSets($pid);
            $output .= $romeo->showRomeo();
            $output .= '<br />';
            $output .= drupal_get_form('scholar_add_ir_datastream_form', $pid);

        }
        $object = $objectHelper->get_datastreams_list_asSimpleXML($pid);
        if (!isset ($object)) {
            drupal_set_message(t("No datastreams available"));
            return ' ';
        }
        $hasOBJStream = null;
        $dataStreamBody = "<br /><table>\n";
        $dataStreamBody .= '<tr><th colspan="4"><h3>' . t("Technical Details") . '</h3></th></tr>';
        foreach ($object as $datastream) {
            foreach ($datastream as $datastreamValue) {
                if ($datastreamValue->ID == 'OBJ') {
                    $hasOBJStream = '1';
                    $mainStreamLabel = $datastreamValue->label;
                    $mainStreamLabel = str_replace("_", " ", $mainStreamLabel);

                }
                //create the links to each datastream
                $dataStreamBody .= $objectHelper->create_link_for_ds($pid, $datastreamValue); 

            }
        }
        global $base_url;
        $dataStreamBody .= "</table>\n";
        if (0 < $user->uid){
            $dataStreamBody.='<br /><a href="'.$base_url.'/fedorair/ir_edit_refworks/'.$pid.'">Edit Metadata</a>';
            $dataStreamBody.='<br /><a href="'.$base_url.'/fedora/repository/'.$pid.'">Manage Object</a>';
        }
        $fieldset = array (
            '#title' => t("Technical Details"
            ), '#collapsible' => TRUE, '#collapsed' => TRUE, '#value' => $dataStreamBody);
        $output .= '<br />' . theme('fieldset', $fieldset);

        return $output;

    }

    function addStreamFormSubmit($form_id, $form_values) {
        module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
        module_load_include('php', 'Fedora_Repository', 'ObjectHelper');

        $types = array (
            //'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
    'application/vnd.oasis.opendocument.text',
            //'application/vnd.openxmlformats-officedocument.presentationml.presentation',
    'text/rtf',
            'application/rtf',
            'application/msword',
            'application/vnd.ms-powerpoint',
            'application/pdf'
        );
        global $user;
        $dsid = 'DOC';
            /* TODO Modify the validators array to suit your needs.
             This array is used in the revised file_save_upload */

        $fileObject = file_save_upload('file_uploaded');
        if (!in_array($fileObject->filemime, $types)) {
            drupal_set_message(t('The detected mimetype %s is not supported', array (
                '%s' => $fileObject->filemime
                    )), 'error');
            return false;
        }
        file_move($fileObject->filepath, 0, 'FILE_EXISTS_RENAME');
        $objectHelper = new ObjectHelper();
        $test = null;
        if ("application/pdf" == $fileObject->filemime) { //do not convert to pdf
            $test = $objectHelper->addStream($form_values['pid'], 'OBJ', $fileObject, true);
            if ($test) {
                $this->updateRefworksStream($form_values['pid'], $form_values['version'], $form_values['usage']);
                drupal_set_message(t('Successfully added PDF file to record.'));
                return true;
            }
        } else {
            $test = $objectHelper->addStream($form_values['pid'], $form_values['version'], $fileObject, true);
        }
        if ($test) { //in ingest successfull convert to pdf and add datastream
            $xmlString = 'requestXML=<?xml version="1.0"?><submission><repository><username>' . $user->name .
            '</username><password>' . $user->pass . '</password><host>' . variable_get('fedora_base_url', 'http://localhost:8080/fedora') .
            '</host></repository><pids><pid>' . $form_values['pid'] . '</pid></pids><dsid>' . $form_values['version'] . '</dsid><collection>/opt/ruleengine/pdfconverter/</collection></submission>';
            //path to ruleengine framework
            //may want to promote this to the db at some point
            $urlFile = drupal_get_path('module', 'scholar') . '/ruleengine_url.txt';
            $url = file_get_contents($urlFile);
            //$url = '137.149.66.158:8080/RuleEngineServlet/RuleEngine';
            $returnValue = do_curl($url,1,1,$xmlString);//$objectHelper->doCurl($url, 1, 1, $xmlString);
            $test = $this->parseReturnValue($returnValue); //did add datastream succeed.
            if ($test) {
                $this->updateRefworksStream($form_values['pid'], $form_values['version'], $form_values['usage']);
            }
            drupal_set_message(t($returnValue));

        } else {
            drupal_set_message(t("Error adding file to IR record!  You may not have permission to modify this record."), 'error');
            watchdog(t("SCHOLAR"),t($returnValue),null,WATCHDOG_ERROR);

            return false;
        }
        return true;
    }


    function buildRefworksEditForm($pid) {
        //module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
        require_once (drupal_get_path('module', 'Fedora_Repository') . '/ObjectHelper.php');

        $object = new ObjectHelper();
        $refworks = $object->getStream($pid, 'refworks', 0);
        $doc = new DomDocument();
        $sourceType = array ();
        $sourceType['Print'] = 'Print';
        $sourceType['Electronic'] = 'Electronic';
        $statuses=array();
        $statuses['PUBLISHED']='Published';
        $statuses['NO_OBJ']='No Text Available';
        $statuses['POST-PRINT']='Post-Print';
        $statuses['PRE-PRINT']='Pre-Print';
        $statuses['OTHER']='Other/Unpublished';
        $usages=array();
        $usages['Contact Author']='Contact Author';
        $usages['Contact Publisher']='Contact Publisher';
        $usages['Creative Commons License by <a href="http://creativecommons.org/licenses/by-nc-nd/3.0/deed.en_CA" target="_blank">(by-nc-nd)</a>']='Creative Commons';
        $types = array ();
        $typesFile = drupal_get_path('module', 'scholar') . '/types.txt';
        $typesList = file($typesFile, FILE_IGNORE_NEW_LINES);
        foreach ($typesList as $type) {
            $t = explode(";", $type);
            $types[$t[0]] = $t[1];
        }
        if (!isset ($refworks)) {
            drupal_set_message(t('Error getting refworks metadata stream'), 'error');
            return null;
        }
        $xml = new SimpleXMLElement($refworks);

        $authors = '';
        foreach ($xml->xpath('//a1') as $author) {
            $authors .= $author . '; ';
        }
        $rt = $xml->xpath('//rt');
        $form = array ();
        $form['rt'] = array (
            '#type' => 'select',
            '#options' => $types,
            '#default_value' => $rt,
            '#title' => t('Type - rt'
            ));
        $sr = $xml->xpath('//sr');
        $form['pid'] = array (
            '#type' => 'hidden',
            '#value' => $pid,


        );

        $form['sr'] = array (
            '#type' => 'select',
            '#options' => $sourceType,
            '#default_value' => $sr,
            '#title' => t('Source Type - sr'
            ));



        $form['a1'] = array (
            '#type' => 'textarea',
            '#required' => true,
            '#default_value' => $authors,
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Authors - a1'
            ));

        $titles = '';
        foreach ($xml->xpath('//t1') as $title) {
            $string=strip_tags($title->asXML());
            $string=html_entity_decode($string);
            $titles .= $string .' ';
        }
        $form['t1'] = array (
            '#type' => 'textarea',
            '#required' => true,
            '#default_value' => $titles,
            '#title' => t('Primary Title - t1'
            ));

        $jfs = '';
        foreach ($xml->xpath('//jf') as $jf) {
            $string=strip_tags($jf->asXML());
            $string=html_entity_decode($string);
            $jfs .= $string . ' ';
        }
        $form['jf'] = array (
            '#type' => 'textarea',
            '#default_value' => $jfs,
            '#title' => t('Periodical Full - jf'
            ));

        $a2 = '';
        foreach ($xml->xpath('//a2') as $a2) {
            $a2s .= $a2 . '; ';
        }

        $form['a2'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($a2s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Secondary Authors - a2'
            ));

        $t2 = '';
        foreach ($xml->xpath('//t2') as $t2) {
            $t2s .= $t2 . ' ';
        }
        $form['t2'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($t2s),
            '#title' => t('Secondary Title - t2'
            ));


        $jos = '';
        foreach ($xml->xpath('//jo') as $jo) {
            $string=strip_tags($jo->asXML());
            $string=html_entity_decode($string);
            $jos .= $string . ' ';
        }
        $form['jo'] = array (
            '#type' => 'textarea',
            '#default_value' => $jos,
            '#title' => t('Periodical Abbrev - jo'
            ));

        $yrs = '';
        foreach ($xml->xpath('//yr') as $yr) {
            $yrs .= $yr . ' ';
        }
        $form['yr'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($yrs),
            '#title' => t('Publication Year - yr'
            ));

        $fds = '';
        foreach ($xml->xpath('//fd') as $fd) {
            $fds .= $fd . ' ';
        }
        $form['fd'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($fds),
            '#title' => t('Publication Date (free form) - fd'
            ));

        $vos = '';
        foreach ($xml->xpath('//vo') as $vo) {
            $vos .= $vo . ' ';
        }
        $form['vo'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($vos),
            '#title' => t('Volume - vo'
            ));
        $iss = '';
        foreach ($xml->xpath('//is') as $is) {
            $iss .= $is . ' ';
        }
        $form['is'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($iss),
            '#title' => t('Issue - is'
            ));
        $sps = '';
        foreach ($xml->xpath('//sp') as $sp) {
            $sps .= $sp . ' ';
        }
        $form['sp'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($sps),
            '#title' => t('Start Page - sp'
            ));

        $ops = '';
        foreach ($xml->xpath('//op') as $op) {
            $ops .= $op . ' ';
        }
        $form['opTemp'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($ops),
            '#title' => t('End Page - op'
            ));


        $pb = '';
        foreach ($xml->xpath('//pb') as $pb) {
            $pbs .= $pb . ' ';
        }
        $form['pb'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($pbs),
            '#title' => t('Publisher - pb'
            ));

        $pps = '';
        foreach ($xml->xpath('//pp') as $pp) {
            $pps .= $pp . ' ';
        }

        $form['pp'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($pps),
            '#title' => t('Place of Publication - pp'
            ));

        $abs = '';
        foreach ($xml->xpath('//ab') as $ab) {
            $string=strip_tags($ab->asXML());
            $string=html_entity_decode($string);
            $abs .= $string . ' ';
        }
        $form['ab'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($abs),
            '#title' => t('Abstract - ab'
            ));

        $nos = '';
        foreach ($xml->xpath('//no') as $no) {
            $string=strip_tags($no->asXML());
            $string=html_entity_decode($string);
            $nos .= $string. ' ';
        }
        $form['no'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($nos),
            '#title' => t('Notes - no'
            ));


        $status = $xml->xpath('//status');
        $form['status'] = array (
            '#type' => 'select',
            '#options' => $statuses,
            '#default_value' => $status,
            '#title' => t('Status')

        );


        $usage = $xml->xpath('//usage');

        $form['usage'] = array (
            '#type' => 'select',
            '#options' => $usages,
            '#default_value' => $usage,
            '#title' => t('Usage')

        );

        $u1s = '';
        foreach ($xml->xpath('//u1') as $u1) {
            $u1s .= $u1 . '; ';
        }
        $form['u1'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($u1s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('UPEI Username - u1'
            ));

        $u2s = '';
        foreach ($xml->xpath('//u2') as $u2) {
            $u2s .= $u2 . '; ';
        }
        $form['u2'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($u2s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('UPEI Departments - u2'
            ));
	
	$form['hidden'] = array(
	    '#type' => 'fieldset',
	    '#title' => t('Less frequently used elements'),
	    '#collapsible' => TRUE,
	    '#collapsed' => TRUE,
	);

        $k1s = '';
        foreach ($xml->xpath('//k1') as $k1) {
            $string=strip_tags($k1->asXML());
            $string=html_entity_decode($string);
            $k1s .= $string . '; ';
        }

        $form['hidden']['k1'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($k1s),
            '#title' => t('Keywords/Descriptors - k1'
            ));



        $ed = '';
        foreach ($xml->xpath('//ed') as $ed) {
            $eds .= $ed . ' ';
        }
        $form['hidden']['ed'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($eds),
            '#title' => t('Edition - ed'
            ));


        $a3s = '';
        foreach ($xml->xpath('//a3') as $a3) {
            $a3s .= $a3 . '; ';
        }
        $form['hidden']['a3'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($a3s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Tertiary Authors - a3'
            ));

        $a4s = '';
        foreach ($xml->xpath('//a4') as $a4) {
            $a4s .= $a4 . '; ';
        }

        $form['hidden']['a4'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($a4s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Quaternary Authors - a4'
            ));

        $a5s = '';
        foreach ($xml->xpath('//a5') as $a5) {
            $a5s .= $a5 . '; ';
        }
        $form['hidden']['a5'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($a5s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Quinary Authors - a5'
            ));

        $t3s = '';
        foreach ($xml->xpath('//t3') as $t3) {
            $t3s .= $t3 . ' ';
        }
        $form['hidden']['t3'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($t3s),

            '#title' => t('Tertiary Title - t3'
            ));

        $sns = '';
        foreach ($xml->xpath('//sn') as $sn) {
            $sns .= $sn . ' ';
        }

        $form['hidden']['sn'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($sns),
            '#title' => t('ISSN/ISBN - sn'
            ));

        $avs = '';
        foreach ($xml->xpath('//av') as $av) {
            $avs .= $av . ' ';
        }
        $form['hidden']['av'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($avs),
            '#title' => t('Availiablity - av'
            ));

        $ads = '';
        foreach ($xml->xpath('//ad') as $ad) {
            $ads .= $ad . ' ';
        }

        $form['hidden']['ad'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($ads),
            '#title' => t('Author Address - ad'
            ));

        $ans = '';
        foreach ($xml->xpath('//an') as $an) {
            $ans .= $an . ' ';
        }
        $form['hidden']['an'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($ans),
            '#title' => t('Accession Number - an'
            ));

        $las = '';
        foreach ($xml->xpath('//la') as $la) {
            $las .= $la . ' ';
        }
        $form['hidden']['la'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($las),
            '#title' => t('Language - la'
            ));

        $cls = '';
        foreach ($xml->xpath('//cl') as $cl) {
            $cls .= $cl . ' ';
        }
        $form['hidden']['cl'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($cls),
            '#title' => t('Classification - cl'
            ));

        $sfs = '';
        foreach ($xml->xpath('//sf') as $sf) {
            $sfs .= $sf . ' ';
        }
        $form['hidden']['sf'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($sfs),
            '#title' => t('Subfile/Database - sf'
            ));

        $ots = '';
        foreach ($xml->xpath('//ot') as $ot) {
            $ots .= $ot . ' ';
        }
        $form['hidden']['ot'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($ots),
            '#title' => t('Original Foreign Title - ot'
            ));

        $lks = '';
        foreach ($xml->xpath('//lk') as $lk) {
            $lks .= $lk . '; ';
        }
        $form['hidden']['lk'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($lks),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Links - lk'
            ));

        $dos = '';
        foreach ($xml->xpath('//do') as $do) {
            $dos .= $do . ' ';
        }
        $form['hidden']['do'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($dos),
            '#title' => t('Document Object Identifier - do'
            ));

        $cns = '';
        foreach ($xml->xpath('//cn') as $cn) {
            $cns .= $cn . ' ';
        }
        $form['hidden']['cn'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($cns),
            '#title' => t('Call Number - cn'
            ));

        $dbs = '';
        foreach ($xml->xpath('//db') as $db) {
            $dbs .= $db . ' ';
        }
        $form['hidden']['db'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($dbs),
            '#title' => t('Database - db'
            ));

        $dss = '';
        foreach ($xml->xpath('//ds') as $ds) {
            $dss .= $ds . ' ';
        }
        $form['hidden']['ds'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($dss),
            '#title' => t('Accession Number - an'
            ));

        $ips = '';
        foreach ($xml->xpath('//ip') as $ip) {
            $ips .= $ip . ' ';
        }
        $form['hidden']['ip'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($ips),
            '#title' => t('Identifying Phrase - ip'
            ));

        $rds = '';
        foreach ($xml->xpath('//rd') as $rd) {
            $rds .= $rd . ' ';
        }
        $form['hidden']['rd'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($rds),
            '#title' => t('Retrieved Data - rd'
            ));

        $sts = '';
        foreach ($xml->xpath('//st') as $st) {
            $sts .= $st . ' ';
        }
        $form['hidden']['st'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($sts),
            '#title' => t('Shortened Title - st'
            ));
        
        $u3s = '';
        foreach ($xml->xpath('//u3') as $u3) {
            $u3s .= $u3 . '; ';
        }
        $form['hidden']['u3'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($u3s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Unused - u3'
            ));

        $u4s = '';
        foreach ($xml->xpath('//u4') as $u4) {
            $u4s .= $u4 . '; ';
        }
        $form['hidden']['u4'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($u4s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Unused - u4'
            ));

        $u5s = '';
        foreach ($xml->xpath('//u5') as $u5) {
            $u5s .= $u5 . '; ';
        }
        $form['hidden']['u5'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($u5s),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('Unused - u5'
            ));

        $uls = '';
        foreach ($xml->xpath('//ul') as $ul) {
            $uls .= $ul . '; ';
        }
        $form['hidden']['ul'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($uls),
            '#description' => t('Separate multiple entries with a semicolon'),
            '#title' => t('URL - ul'
            ));

        $sls = '';
        foreach ($xml->xpath('//sl') as $sl) {
            $sls .= $sl . ' ';
        }
        $form['hidden']['sl'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($sls),
            '#title' => t('Sponsoring Library - sl'
            ));

        $lls = '';
        foreach ($xml->xpath('//ll') as $ll) {
            $lls .= $ll . ' ';
        }
        $form['hidden']['ll'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($lls),
            '#title' => t('Sponsoring Library Location - ll'
            ));

        $crs = '';
        foreach ($xml->xpath('//cr') as $cr) {
            $crs .= $cr . ' ';
        }
        $form['hidden']['cr'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($crs),
            '#title' => t('Cited Reference - cr'
            ));

        $wts = '';
        foreach ($xml->xpath('//wt') as $wt) {
            $wts .= $wt . ' ';
        }
        $form['hidden']['wt'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($wts),
            '#description' => 'This field does not display.  Put this info in the Primary Title Field.',
            '#title' => t('Website Title - wt'
            ));

        $a6s = '';
        foreach ($xml->xpath('//a6') as $a6) {
            $a6s .= $a6 . '; ';
        }
        $form['hidden']['a6'] = array (
            '#type' => 'textarea',
            '#default_value' => html_entity_decode($a6s),
            '#description' => t('This field does not display.  Put this info in the Authos or Secondary Authers'),
            '#title' => t('Website Editor - a6'
            ));

        $wvs = '';
        foreach ($xml->xpath('//wv') as $wv) {
            $wvs .= $wv . ' ';
        }
        $form['hidden']['wv'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($wvs),
            '#title' => t('Website Version - wv'
            ));

        $wps = '';
        foreach ($xml->xpath('//wp') as $wp) {
            $wps .= $wp . ' ';
        }
        $form['hidden']['wp'] = array (
            '#type' => 'textfield',
            '#default_value' => html_entity_decode($wps),
            '#title' => t('Date of Electronic Publication - wp'
            ));
	
	$form['submit'] = array (
            '#type' => 'submit',
            '#value' => 'Update',
            //'#suffix' => 'Less frequently used elements below<br /><hr />'
        );

        return $form;

    }


    function updateRefworksMetaData($form_values){
        $refworksXML=$this->createRefworksFromForm($form_values);
	$dc = $this->updateDCFromReference($refworksXML,$form_values['pid']);
        $this->updateRefworksStream($form_values['pid'],null,null,$refworksXML);
    }

    function createManyElementsFromString(&$reference,$key,$value,&$dom){
        $elements = explode(';',$value);
        foreach($elements as $element){
            if($element!=''){
                $reference->appendChild($dom->createElement($key,htmlentities($element)));
            }
        }
    }

    function createRefworksFromForm($form_values){
        module_load_include('inc','Fedora_Repository', 'api/fedora_utils');
        $dom = new DomDocument("1.0","UTF-8");
        $dom->substituteEntities = FALSE;
        $dom->formatOutput = true;
        $reference = $dom->createElement("reference");
        $dom->appendChild($reference);
        foreach($form_values as $key => $value){
            $value=htmlspecialchars(trim($value),ENT_NOQUOTES,'UTF-8');
            if ($key == 'op'){
            break;
            }
            if ($key == 'opTemp'){
                $key = 'op';
             } 
            if($key!='form_token'&&$key!='form_id'&&$key!='submit'&&$key!='pid'&&$value!=''){
                if($key=='a1'||$key=='u1'||$key=='u2'||$key=='a2'||$key=='a3'||$key=='op'||
                    $key=='a4'||$key=='a5'||$key=='a6'||$key=='lk'||$key=='k1'||
                    $key=='ul'){
                    $this->createManyElementsFromString($reference,$key,$value,$dom);
                }else{
                    $reference->appendChild($dom->createElement($key,$value));
                }
            }

        }
        return $dom->saveXML();


    }

    function updateDCFromReference($xmlStream,$pid){
        $dom = new DomDocument("1.0", "UTF-8");
        ///begin writing qdc
        $oai = $dom->createElement("oai_dc:dc");
        $oai->setAttribute('xmlns:oai_dc', "http://www.openarchives.org/OAI/2.0/oai_dc/");
        $oai->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
        $oai->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
        $oai->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
        $dom->appendChild($oai);
        //dc elements


        try {
            $dcdom = new DOMDocument('1.0', 'UTF-8');
            $dcdom->substituteEntities = FALSE;
            $dcdom->loadXML($xmlStream);
            $reference = simplexml_import_dom($dcdom);
            //$reference = new SimpleXMLElement($xmlStream);

        } catch (Exception $e) {
            drupal_set_message(t('Error Processing Refworks File! ' . $e->getMessage()));
            return FALSE;
        }
        //$reference = $xml->reference;
        foreach ($reference->sr as $value) {
            $element = $dom->createElement('dc:type', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->id as $value) {
            $identifier = $value;
        }
        foreach ($reference->a1 as $value){
            $element = $dom->createElement('dc:creator', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }

        foreach ($reference->t1 as $value) {
            $form_values['dc:title'] = $value;
            $element = $dom->createElement('dc:title', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->jf as $value) {
            $source = $value;
        }
        foreach ($reference->yr as $value) {
            $element = $dom->createElement('dc:date', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->vo as $value) {
            $source .= ' Volume: ' . $value;
        }
        foreach ($reference->is as $value) {
            $source .= ' Issue: ' . $value;
        }
        foreach ($reference->sp as $value) {
            $source .= ' Start Page: ' . $value;
        }
        foreach ($reference->op as $value) {
            $source .= ' Other Pages: ' . $value;
        }
        foreach ($reference->ul as $value) {
            $source .= ' URL: ' . $value;
        }
        foreach ($reference->k1 as $value) {
            $element = $dom->createElement('dc:subject', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->a2 as $value) {
            $element = $dom->createElement('dc:contributor', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->a3 as $value) {
            $element = $dom->createElement('dc:contributor', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->a4 as $value) {
            $element = $dom->createElement('dc:contributor', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->a5 as $value) {
            $element = $dom->createElement('dc:contributor', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->la as $value) {
            $element = $dom->createElement('dc:language', htmlspecialchars($value,ENT_NOQUOTES,'UTF-8'));
            $oai->appendChild($element);
        }
        foreach ($reference->pb as $value) {
            $source = 'Publisher: ' . $value;
        }
        foreach ($reference->pp as $value) {
            $source .= ' Place of Publication: ' . $value;
        }
        foreach ($reference->sn as $value) {
            $identifier .= ' ISSN/ISBN: ' . $value;
            $this->romeoUrlString = $this->romeoUrlString . $value;
        }
        foreach ($reference->ab as $value) {
            $description = ' abstract: ' . $value;
        }
        foreach ($reference->cr as $value) {
            $description .= ' Cited Reference: ' . $value;
        }
        $element = $dom->createElement('dc:description', htmlspecialchars($description,ENT_NOQUOTES,'UTF-8'));
        $oai->appendChild($element);
        $element = $dom->createElement('dc:source', htmlspecialchars($source,ENT_NOQUOTES,'UTF-8'));
        $oai->appendChild($element);
        $element = $dom->createElement('dc:identifier', htmlspecialchars($idenitifier,ENT_NOQUOTES,'UTF-8'));
        $oai->appendChild($element);
        //$dom->appendChild($datastream);


        require_once (drupal_get_path('module', 'Fedora_Repository') . '/ObjectHelper.php');
        $object = new ObjectHelper();
        $test = $dom->saveXML();

        $params = array (
            "pid" => $pid,
            "dsID" => 'DC',
            "altIDs" => "",
            "dsLabel" => "Default Dublin Core Record",
            "MIMEType" => "text/xml",
            "formatURI" => "URL",
        "dsContent" => $dom->saveXML(), "checksumType" => "DISABLED", "checksum" => "none", "logMessage" => "dc_datastream_modified", "force" => "true");
        return $object->modifyDatastreamByValue($params);

    }


    //after the ruleengine framework adds the converted datastream we modify the refworks xml datastream
    //so we know that about the pdf
    function updateRefworksStream($pid, $version = null, $usage = null,$xmlString=null) {
        module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
        $object = new ObjectHelper();
        if(!isset($xmlString)){
            $refworks = $object->getStream($pid, 'refworks', 0);
        }else{
            $refworks = $xmlString;
        }
        $doc = new DOMDocument('1.0', 'UTF-8');
        $doc->substituteEntities = FALSE;
        try {
            $doc->loadXML($refworks);
        } catch (exception $e) {
            drupal_set_message(t('Error loading Refworks info! ') . $e->getMessage());
            return ' ';
        }
        if (isset ($usage)) {
            $usageNode = $doc->createElement('usage', $usage);
            $nodeList = $doc->getElementsByTagName('reference');
            foreach ($nodeList as $reference) {
                $reference->appendChild($usageNode);
            }
        }
        if (isset ($version)) {
            $nodeList = $doc->getElementsByTagName('status');
            if($nodeList->length>0){
                foreach ($nodeList as $node) {
                    $node->nodeValue = $version; //should only be one
                }
            }else{
                //create the useage element as it does not exist
                $statusNode=$doc->createElement('status',$version);
                //add it to the reference node
                $reference_node_list = $doc->getElementsByTagName('reference');
                foreach ($reference_node_list as $reference) {
                    $reference->appendChild($statusNode);
                }

            }
        }
        $params = array (
            "pid" => $pid,
            "dsID" => 'refworks',
            "altIDs" => "",
            "dsLabel" => "refworks record",
            "MIMEType" => "text/xml",
            "formatURI" => "URL",
        "dsContent" => $doc->saveXML(), "checksumType" => "DISABLED", "checksum" => "none", "logMessage" => "refworks_datastream_modified", "force" => "true");
        $object->modifyDatastreamByValue($params);

    }
    //parses the return from the ruleengine framework
    function parseReturnValue(& $input) {
        $doc = new DOMDocument();
        $doc->loadXML($input);
        $nodeList = $doc->getElementsByTagName('message');
        foreach ($nodeList as $node) {
            if ("Document conversion complete." == $node->nodeValue) {
                $input = $node->nodeValue;
                return true;
            }
        }

        $input = "Error during file conversion!";
        return false;

    }
    //associates roles with users.  gets roles from refworks field and creates roles if necassary
    function get_and_create_roles(){
      global $user;
      if($user->id != 1){
        return 'access denied';
      }
         module_load_include('php', 'scholar', 'IRSearchClass');
          $searchClass = new IRSearchClass();
          
          $global_roles = user_roles();
        $result = db_query('SELECT users.uid AS uid,users.name AS users_name FROM users users where users.uid>20');
        while ($user = db_fetch_object($result)) {            
            $user_roles = array();
            $xml = $searchClass->custom_search_return_rawxml("refworks.u1:$user->users_name");
            $doc = new DOMDocument();
           
            $doc->loadXML($xml);
            $xPath = new DOMXPath($doc);
        //add users to department list.  This is a hack as not all users will be in dupal
            $nodeList = $xPath->query('//field[@name="refworks.u2"]');
           
            foreach($nodeList as $node) {
                if(!in_array($node->nodeValue,$global_roles)){
                    $global_roles[]=$node->nodeValue;//add it to the array so we don't duplicate
                    db_query("INSERT INTO {role} (name) VALUES ('%s')", $node->nodeValue);//create the role
                }
                if(!in_array($node->nodeValue,$user_roles)) {
                    $roles_table = user_roles(); // loads the roles table into an array

                    $rid = array_search($node->nodeValue, $roles_table);
                    $test=$node->nodeValue;
                    
                    $user_roles[$rid] = $node->nodeValue;
                }
            }
            if(!empty($user_roles)){
                $this->add_roles_to_user($user->uid,$user_roles);
            }
        }



    }

    function add_roles_to_user($user_uid,$user_roles){
           $myuser = module_invoke('user', 'load', $user_uid);
            //$current_roles = $myuser->roles;
            //if(isset($current_roles)){
            //    $user_roles = array_merge($current_roles,$user_roles);
            //}
          
            module_invoke('user', 'save', $myuser, array('roles' => $user_roles));
    }


/*function browse_by_name_form($role = null, $userNameArray = null) {
  module_load_include('inc','Fedora_Repository', 'api/fedora_utils');
    $usersFile = drupal_get_path('module', 'scholar') . '/FacultyStaff.csv';
    $userList = file($usersFile, FILE_IGNORE_NEW_LINES);

    $facultyArray = array ();
    $users = array ();
    foreach ($userList as $faculty) {
        $f = explode(",", $faculty);
        $userName = $f[2];
        $userName = fix_encoding((trim($userName)));
        if ($userNameArray != null) {
          //$userName = strtolower($f[2]);
            if (in_array($userName, $userNameArray)) {
                $users[$userName] =fix_encoding($f[1]);
            }
        } else {
           // $userName = strtolower($f[2]);
            $users[$userName] = fix_encoding($f[1]);
        }
    }

    $form = array ();

    $form['user']['user_id'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#type' => 'select',
            '#options' => $users,


    );
    $form['submit'] = array (
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#type' => 'submit',
            '#value' => t('Browse'
            ),);
            return $form;
}*/

    function browse_by_name_form($role = null) {
        $result = null;
        if (!$role) {
            $result = db_query('SELECT users.uid FROM {users} where users.uid > %d order by users.name', 0);
        } else {
            $result = db_query("SELECT users.uid FROM {users},{users_roles},{role} where users_roles.rid=role.rid and role.name='$role' and users_roles.uid = users.uid and users.uid > %d order by users.name", 0);
        }
        $users = array ();
        $fileName = drupal_get_path('module', 'scholar') . '/userExclusionList.txt';
        $exclusionList = file_get_contents($fileName);
        $exclusionList = explode(",",$exclusionList);
        while ($user_id = db_fetch_object($result)) {
            $user = user_load(array (
 'uid' => $user_id->uid
                ));
            if(!in_array($user->name,$exclusionList)){

                if ($user->uid) {
                    //$users[$user->uid] = $user->profile_name.' ('.$user->name.') ';
                    $users[$user->name] = $user->profile_name;//.' ('.$user->name.') ';
                }
            }
        }
        asort($users);

        $form = array ();

        $form['user']['user_id'] = array (
 '#prefix' => '<td>',
 '#suffix' => '</td>',
 '#type' => 'select',
 '#options' => $users,

        );
        $form['submit'] = array (
 '#prefix' => '<td>',
 '#suffix' => '</td>',
 '#type' => 'submit',
 '#value' => t('Browse'
            ),);
        return $form;
    }

    function browse_by_department() {
        module_load_include('inc','Fedora_Repository', 'api/fedora_utils');
        $fileName = drupal_get_path('module', 'scholar') . '/inclusionList.txt';
        $inclusionList = file_get_contents($fileName);
        $inclusionList = fix_encoding($inclusionList);
        $inclusionList = explode(",", $inclusionList);
        $ingnoreRolesLessThen = 2;
        $result = db_query('SELECT role.rid, role.name FROM {role} where role.rid > %d order by role.name', 0);
        $output .= '<ul>';
        while ($role = db_fetch_object($result)) {

            if ($role->rid > $ignoreRolesLessThen && in_array($role->name, $inclusionList)) {
                $urlPortion = urlencode($role->name);
                $name = $role->name;
                if ('Home Economics' == $name) {
                    $name = "Family and Nutritional Sciences";
                } else
                if ('Anatomy Physiology' == $name) {
                    $name = "Biomedical Sciences";
                } else
                if ('Womens Studies' == $name) {
                    $name = "Women's Studies";

                } else
                if ('Path Micro' == $name) {
                    $name = "Pathology and Microbiology";

                } else
                if ('Soc Anth' == $name) {
                    $name = "Sociology and Anthropology";
                }
                $output .= "<li><a href=ir_role/refworks.u2/$urlPortion>" . $name . '</a></li>';
            }
        }
        $output .= '</ul>';

        return $output;
    }

    function build_add_datastream_form($pid) {
        $form = array ();
        $certifyBox = array (
            '1' => 'I Certify That I have the right to upload this version.'
        );
        $usage = array (
            'Contact Publisher' => 'Contact Publisher (I do not hold the copyright)',
            'Contact Author' => 'Contact Author (I hold the copyright and wish to retain all rights)',
            //'Creative Commons License by <a href="http://creativecommons.org/licenses/by-nc-nd/3.0/deed.en_CA" target="_blank">(by-nc-nd)</a>' => 'I hold the copyright and wish to assign the Creative Commons <a href="http://creativecommons.org/licenses/by-nc-nd/3.0/deed.en_CA" target="_blank">(by-nc-nd)</a> License to this item'
        );
        $versions = array (
            'PRE-PRINT' => 'PRE-PRINT',
            'POST-PRINT' => 'POST-PRINT',
            'PUBLISHED' => 'PUBLISHED PDF',
            'OTHER' => 'OTHER/UNPUBLISHED'
        );
        //$form['#attributes']['enctype'] = 'multipart/form-data';
        $form['#attributes'] = array('enctype' => "multipart/form-data");
        $form['file']['file_uploaded'] = array (
            '#type' => 'file',
            '#title' => t('Upload Document'
            ), '#size' => 48,
            //'#required'=>'true',
    '#description' => t('Full text'));
        $form['file']['version'] = array (
            '#type' => 'radios',
            '#title' => t('Document Version'
            ), '#options' => $versions, '#required' => 'true',
            //'#description'=>t('The version of the document you are uploading.')
        );
        $form['file']['usage'] = array (
            '#type' => 'radios',
            '#title' => t('Use Permission'
            ), '#options' => $usage, '#required' => 'true',);
        $form['file']['certify'] = array (
            '#type' => 'checkboxes',
            '#title' => t('I Certify'
            ), '#options' => $certifyBox, '#required' => 'true',
            //'#description'=>t('The version of the document you are uploading.')
        );

        $form['pid'] = array (
            '#type' => 'hidden',
            '#value' => $pid
        );
        $form['submit'] = array (
            '#type' => 'submit',
            '#value' => t('Upload'
            ), '#suffix' => t('Please be patient.  Document upload and conversion can take up to a few minutes.'));
        return $form;
    }
}
?>
