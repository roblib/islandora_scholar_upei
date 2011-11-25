<?php
class IRSearchClass {
  public static $ADVANCED_SEARCH_NUMBER_FIELDS = 5;
  function quickSearch($type, $query,$showForm=1,$orderBy=0,& $userArray) {
    module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
    if (user_access('view fedora collection')) {
      $numberOfHistPerPage = '5000';//hack for IR they do not want next button
      $luceneQuery = null;
      //demo search string ?operation=gfindObjects&indexName=DemoOnLucene&query=fgs.DS.first.text%3Achristmas&hitPageStart=11&hitPageSize=10
      $keywords = explode(' ', $query);

      foreach ($keywords as $keyword) {
        $luceneQuery .= $type . ':' . fix_encoding($keyword). '+AND+';
      }
      $luceneQuery = substr($luceneQuery, 0, strlen($luceneQuery) - 5);
      
      $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
      //$keys = htmlentities(urlencode($query));
      $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
      $searchString = '?operation=gfindObjects&indexName=' . $indexName . '&restXslt=copyXml&query=' . $luceneQuery;
      $searchString .= '&hitPageSize='.$numberOfHistPerPage.'&hitPageStart=1';
      //$searchString = htmlentities($searchString);
      $searchUrl .= $searchString;

     // $objectHelper = new ObjectHelper();
 
      $resultData = do_curl($searchUrl,1);//$objectHelper->doCurl($searchUrl,1);

      if(isset($userArray)&&isset($resultData)) {
        $doc = new DOMDocument();
        $doc->loadXML($resultData);
        $xPath = new DOMXPath($doc);
        //add users to department list.  This is a hack as not all users will be in dupal
        $nodeList = $xPath->query('//field[@name="refworks.u1" or @name="u1"]');
        foreach($nodeList as $node) {
          if(!in_array($node->nodeValue,$userArray)) {
            $userArray[]=$node->nodeValue;
          }
        }


      }

      if($showForm) {
        $output = '<Strong>Quick Search</strong><br /><table class="table-form"><tr>'.drupal_get_form('scholar_quick_search_form').'</tr></table>';
      }
      $output.=$this->applyXSLT($resultData,$orderBy);
      return $output;

    }

  }

  function custom_search($query) {
    module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
    if (user_access('view fedora collection')) {
      $numberOfHistPerPage = '1000';//hack for IR they do not want next button
      $luceneQuery = null;
      //demo search string ?operation=gfindObjects&indexName=DemoOnLucene&query=fgs.DS.first.text%3Achristmas&hitPageStart=11&hitPageSize=10


      $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
      $query=htmlentities(urlencode($query));
      $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
      $searchString = '?operation=gfindObjects&indexName=' . $indexName . '&restXslt=copyXml&query=' . $query;
      $searchString .= '&hitPageSize='.$numberOfHistPerPage.'&hitPageStart=1';
      //$searchString = htmlentities($searchString);
      $searchUrl .= $searchString;

      $objectHelper = new ObjectHelper();

      $resultData = do_curl($searchUrl,1);
      //var_dump($resultData);exit(0);
      //	$doc = new DOMDocument();
      //	$doc->loadXML($resultData);

      $output.=$this->applyLuceneXSLT($resultData);
      return $output;

    }
  }

  function custom_search_return_rawxml($query) {
    module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
    if (user_access('view fedora collection')) {
      $numberOfHistPerPage = '1000';//hack for IR they do not want next button
      $luceneQuery = null;
      //demo search string ?operation=gfindObjects&indexName=DemoOnLucene&query=fgs.DS.first.text%3Achristmas&hitPageStart=11&hitPageSize=10


      $indexName = variable_get('fedora_index_name', 'DemoOnLucene');
      $query=htmlentities(urlencode($query));
      $searchUrl = variable_get('fedora_fgsearch_url', 'http://localhost:8080/fedoragsearch/rest');
      $searchString = '?operation=gfindObjects&indexName=' . $indexName . '&restXslt=copyXml&query=' . $query;
      $searchString .= '&hitPageSize='.$numberOfHistPerPage.'&hitPageStart=1';
      //$searchString = htmlentities($searchString);
      $searchUrl .= $searchString;

      $objectHelper = new ObjectHelper();

      $resultData = do_curl($searchUrl,1);
      //var_dump($resultData);exit(0);
      //	$doc = new DOMDocument();
      //	$doc->loadXML($resultData);


      return $resultData;

    }
  }


  function applyLuceneXSLT($resultData) {
    $path = drupal_get_path('module', 'Fedora_Repository');
    $proc = null;
    if (!$resultData) {
      drupal_set_message(t('No Results!'));
      return ' '; //no results
    }


    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('Error loading results xslt! ').$e->getMessage())	;
      return ' ';
    }

    //inject into xsl stylesheet
    $proc->setParameter('', 'searchToken', drupal_get_token('fedora_repository_advanced_search')); //token generated by Drupal, keeps tack of what tab etc we are on
    $proc->setParameter('', 'searchUrl', url('search') . '/fedora_repository'); //needed in our xsl
    $proc->setParameter('', 'objectsPage', base_path());
    $proc->setParameter('', 'allowedPidNameSpaces', variable_get('fedora_pids_allowed', 'demo: changeme:'));
    $proc->registerPHPFunctions();
    $xsl = new DomDocument();

    $test= $xsl->load($path . '/xsl/results.xsl');
    if (!isset($test)) {
      drupal_set_message(t('Error loading search results xslt!'));
      return t('Error loading search results xslt! ');
    }

    $input = new DomDocument();
    $didLoadOk = $input->loadXML($resultData);

    if (!isset($didLoadOk)) {
      drupal_set_message(t('Error loading search results!'));
      return t('Error loading search results! ');


    } else {

      $proc->importStylesheet($xsl);

      $newdom = $proc->transformToDoc($input);

      return $newdom->saveXML();


    }



  }

  function applyXSLT($resultData,$orderBy=0) {

    $path = drupal_get_path('module', 'scholar');
    $proc = null;
    if (!$resultData) {
      drupal_set_message(t('No Results!'));
      return ' '; //no results
    }


    try {
      $proc = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('Error loading results xslt! ').$e->getMessage())	;
      return ' ';
    }

    //inject into xsl stylesheet
    //$proc->setParameter('', 'searchToken', drupal_get_token('search_form')); //token generated by Drupal, keeps tack of what tab etc we are on
    $proc->setParameter('', 'searchUrl', url('search') . '/fedora_repository'); //needed in our xsl
    $proc->setParameter('', 'objectsPage', base_path());
    $proc->setParameter('', 'allowedPidNameSpaces', variable_get('fedora_pids_allowed', 'demo: changeme:'));
    $proc->setParameter('', 'orderBy', $orderBy);
    $xsl = new DomDocument();

    $test=$xsl->load($path . '/xsl/results.xsl');
    if (!isset($test)) {
      drupal_set_message(t('Error loading search results xslt!'));
      return t('Error loading search results xslt! ');
    }

    $input = new DomDocument();
    $didLoadOk = $input->loadXML($resultData);


    if (!isset($didLoadOk)) {
      drupal_set_message(t('Error loading search results!'));
      return t('Error loading search results! ');


    } else {

      $xsl = $proc->importStylesheet($xsl);

      $newdom = $proc->transformToDoc($input);

      return $newdom->saveXML();

    }


  }

  function theme_advanced_search_form($form) {
    $repeat = variable_get('fedora_repository_advanced_block_repeat',  t('3'));

    $output .= drupal_render($form['search_type']['type1']) ;
    $output .= drupal_render($form['fedora_terms1']) ;
    $output .=  drupal_render($form['andor1'])  . drupal_render($form['search_type']['type2']) ;
    $output .= drupal_render($form['fedora_terms2']);
    if($repeat>2 && $repeat < 9) {
      for($i=3;$i<$repeat+1;$i++) {
        $t=$i-1;
        $output .=  drupal_render($form["andor$t"])  . drupal_render($form['search_type']["type$i"]) ;
        $output .= drupal_render($form["fedora_terms$i"]) ;
      }

    }

    $output .=  drupal_render($form['submit']) ;
    $output .= drupal_render($form);
    return $output;
  }
  function build_advanced_search_form() {

    $types = $this->get_search_terms_array();

    $andOrArray = array (
        'AND' => 'and',
        'OR' => 'or'
    );
    $form = array ();
    $repeat = variable_get('fedora_repository_advanced_block_repeat',  t('3'));

    $form['search_type']['type1'] = array (
        '#title' => t(''
        ), '#type' => 'select', '#options' => $types);
    $form['fedora_terms1'] = array (
        '#size' =>'24',
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
        '#size' =>'24',
        '#type' => 'textfield',
        '#title' => t(''
    ));
    if($repeat>2 && $repeat < 9) { //eliminate possibility of invalid values
      for($i=3;$i<$repeat+1;$i++) {
        $t=$i-1;
        $form["andor$t"] = array (
            '#title' => t(''
            ), '#type' => 'select', '#default_value' => 'AND', '#options' => $andOrArray);
        $form['search_type']["type$i"] = array (
            '#title' => t(''
            ), '#type' => 'select', '#options' => $types);
        $form["fedora_terms$i"] = array (
            '#size' =>'24',
            '#type' => 'textfield',
            '#title' => t(''
        ));
      }
    }

    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('search'
    ));
    return $form;
  }


  function get_search_terms_array() {
    $path = drupal_get_path('module', 'Fedora_Repository');
    $xmlDoc = new DomDocument();
    $xmlDoc->load($path . '/searchTerms.xml');
    $nodeList = $xmlDoc->getElementsByTagName('term');
    $types = array ();
    for ($i = 0; $i < $nodeList->length; $i++) {
      $field = $nodeList->item($i)->getElementsByTagName('field');
      $value = $nodeList->item($i)->getElementsByTagName('value');
      $fieldValue = $field->item(0)->nodeValue;
      $valueValue = $value->item(0)->nodeValue;
      $types["$fieldValue"] = "$valueValue";


    }

    return $types;

  }
}
?>
