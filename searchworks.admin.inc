<?php
//$Id: searchworks.admin.inc,v 1.4 2010-11-17 23:24:43 jbickar Exp $
/*
 * @file
 * Contains additional administrative functions for searchworks.module
 */
/*
 * Pull the items from Searchworks and get the XML links.
 */
function searchworks_get_feeds() {
//  $url ="http://searchworks.stanford.edu/view.rss?f[access_facet][]=At+the+Library&f[building_facet][]=Cubberley+%28Education%29&f[format][]=Book&per_page=10&sort=pub_date_sort+desc%2C+title_sort+asc";  //re-enable this when going live - jbickar
//  $url = drupal_get_path('module', 'searchworks') .'/test_xml.inc'; // disable this when going live - jbickar
  $url = variable_get('searchworks_url', NULL);
  $data = file_get_contents($url);
  $parser = xml_parser_create();
  xml_parse_into_struct($parser, $data, $values, $index);
  xml_parser_free($parser);
  $links = array();
  foreach($values as $value) {
    if($value['tag'] == 'LINK'){
      $links[] = $value['value'] .".xml"; 
    }
  }
  unset($links[0]);
  return $links;
}

/*
 * Pull the XML for each individual item.
 */
function searchworks_collate_feeds() {
  $links = searchworks_get_feeds();
  $data = '';
  foreach($links as $link) {
    $data .= file_get_contents($link); //re-enable this when going live - jbickar
  }
//  $data = file_get_contents(drupal_get_path('module', 'searchworks') .'/test_xml2.inc'); // disable this when going live - jbickar
  $strip_namespace_string = '/\sxmlns\=\'http\:\/\/www\.loc\.gov\/MARC21\/slim\'/';
  //remove the namespace declaration. Will this cause any problems?
  $data = preg_replace($strip_namespace_string, '', $data);
  return $data;
}

/*
 * Push it out as a feed.
 */
function searchworks_publish_feed() {
  $title = t('SearchWorks from @site_name by date', array('@site_name' => variable_get('site_name', 'this site')));
  $link = url('searchworks/feed', array('absolute' => TRUE));
  $description = t('Latest items from Searchworks at @site_name, sorted by date', array('@site_name' => variable_get('site_name', 'this site')));
  $items = searchworks_collate_feeds();
  $output = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  $output .= format_rss_channel($title, $link, $description, $items);
  drupal_set_header('Content-Type: application/xml; charset=utf-8');
  print $output;
}

function searchworks_page() {
  $output = t("Configure the SearchWorks module here.");
  $url = url('searchworks/feed');
  $title = t('Searchworks from Cubberley by date');
  $strip_namespace_string = '\sxmlns\=\'http\:\/\/www\.loc\.gov\/MARC21\/slim\'';
  drupal_add_feed($url, $title);
  $output .= drupal_get_form('searchworks_settings_form');
  return $output;
}

function searchworks_settings_form() {
  $form['searchworks_settings'] = array(
    '#title' => t('Settings'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE
  );
  $form['searchworks_settings']['searchworks_url'] = array(
    '#title' => t('SearchWorks RSS Feed URL'),
    '#type' => 'textfield',
    '#description' => t('Enter the URL for the RSS feed of a search you have constructed in Searchworks (e.g., ') .'http://searchworks.stanford.edu/view.rss?f[access_facet][]=At+the+Library&f[building_facet][]=Green+%28Humanities+%26+Social+Sciences%29' .')',
    '#required' => TRUE,
    '#maxlength' => 1024,
  );
  
  return system_settings_form($form);
}