<?php
// $Id$

// Netherlands

function location_province_list_nl() {
  return array('DR' => "Drenthe",
    'FL' => "Flevoland",
    'FR' => "Friesland",
    'GE' => "Gelderland",
    'GR' => "Groningen",
    'LI' => "Limburg",
    'NB' => "Noord Brabant",
    'NH' => "Noord Holland",
    'OV' => "Overijssel",
    'UT' => "Utrecht",
    'ZE' => "Zeeland",
    'ZH' => "Zuid Holland");
}

function location_map_link_nl_routenet($location = array()) {
  return (
    'http://www.routenet.nl/address.asp?task=map_address&language=NL'
    . '&zipcode=' . urlencode($location['postal_code'])
    . '&street=' . urlencode($location['street'])
    . '&city=' . urlencode($location['city'])
    . '&country=NL'
  );
}

function location_map_link_nl_yahoo($location = array()) {
  $get_query = '?';
  if (isset($location['street'])) {
    $get_query .= 'addr='. urlencode($location['street']) .'&amp;';
  }
  if ($location['province'] != '' || $location['city'] != '' || $location['postal_code'] != '') {
    $get_query .= 'csz='. _location_nl_yahoo_csz_get_field($location) .'&amp;';
  }
  $get_query .= 'country='. urlencode($location['country']);
  return ('http://maps.yahoo.com/maps_result'. $get_query);
}


function location_map_link_nl_google($location = array()) {
  $query_params = array();

  foreach (array('street', 'city', 'province', 'postal_code', 'country') as $field) {
    if (isset($location[$field])) {
      $query_params[] = $location[$field];
    }
  }

  if (count($query_params)) {
    return ('http://maps.google.com?q='. urlencode(implode(", ", $query_params)));
  }
  else {
    return NULL;
  }
}

function location_map_link_nl_mapquest($location = array()) {
  if (isset($location['street'])) {
    $get_query .= 'address='. urlencode($location['street']) .'&amp;';
  }

  if (isset($location['city'])) {
    $get_query .= 'city='. urlencode($location['city']) .'&amp;';
  }

  if (isset($location['province'])) {
    $get_query .= 'state='. urlencode($location['province']) .'&amp;';
  }

  if (isset($location['postal_code'])) {
    $get_query .= 'zipcode='. urlencode($location['postal_code']);
  }
  if (strlen($get_query)) {
    return 'http://www.mapquest.com/maps/map.adp?searchtype=address&amp;country=US&amp;'. $get_query;
  }
  else {
    return NULL;
  }
}

/**
 * @return
 *   An array where
 *     -> the key is the word that helps identify the name of function that builds the link.  For example, a key of 'yahoo' means the name of the
 *        the function that builds a link to a map on Yahoo! Maps would be 'location_map_link_nl_yahoo'
 *     -> the value is itself an array with 3 key/value pairs:
 *          'name' => points to the name of the mapping service.  For 'yahoo', this would be 'Yahoo! Maps'
 *          'url' => the url of the main page of the mapping service.  For 'yahoo', this would be 'http://maps.yahoo.com'
 *          'tos' => the url of the page that explains the map providers Terms of Service, or Terms of Use. For 'yahoo', this would be
 *                   'http://help.yahoo.com/help/us/maps/maps-24.html'
 */
function location_map_link_nl_providers() {
  return array('google' => array('name' => 'Google Maps', 'url' => 'http://maps.google.com', 'tos' => 'http://www.google.com/help/terms_local.html'),
               'yahoo' => array('name' => 'Yahoo! Maps', 'url' => 'http://maps.yahoo.com', 'tos' => 'http://help.yahoo.com/help/us/maps/maps-24.html'),
               'mapquest' => array('name' => 'MapQuest', 'url' => 'http://www.mapquest.com', 'tos' => 'http://www.mapquest.com/features/main.adp?page=legal'),
               'routenet' => array('name' => 'RouteNet', 'url' => 'http://www.routenet.nl', 'tos' => 'http://www.routenet.nl'),
              );
}

/**
 * @return
 *   An array of values that work as keys to the array returned by location_map_link_us_providers.  The idea is that if the
 *   administrator of the site has not yet had a chance to visit the "Map Links" subtab on the location module's settings page,
 *   that we can provide deep-linking to a relatively safe default.  By 'relatively safe', we mean that the Terms Of Service of
 *   the provider of the maps are flexible enough for most parties.
 *
 *   For the case of the U.S., 'google' has relatively flexible Terms Of Service, whereas Yahoo! Maps and MapQuest have more
 *   restrictive Terms Of Service.
 *
 */
function location_map_link_nl_default_providers() {
  return array('google');
}

function location_geocode_nl_providers() {
  return array(
    'yahoo' => array('name' => 'Yahoo! Maps Web Services', 'url' => 'http://developer.yahoo.com/maps/rest/V1/geocode.html', 'tos' => 'http://developer.yahoo.com/maps/mapsTerms.html'),

    'google' => array('name' => 'Google Maps API', 'url' => 'http://www.google.com/apis/maps/', 'tos' => 'http://www.google.com/apis/maps/terms.html')

  );
}

function location_geocode_nl_yahoo_settings(){
  $form = array();

  $form['location_geocode_us_yahoo_appid'] = array(
    '#type' => 'textfield',
    '#title' => t('Yahoo! Web Services Application ID'),
    '#size' => 64,
    '#maxlength' => 128,
    '#default_value' => variable_get('location_geocode_us_yahoo_appid', 'YahooDemo'),
    '#description' => t('Unless you are using this site to test and develop, you will need to obtain a Yahoo! Web Services Application ID from the %network_link.  If you are using for development and testing purposes, you can use \'YahooDemo\' as your AppID.  When getting an Application ID from Yahoo!, please also be sure to review the %usage_policy.', array('%network_link' => '<a href="http://api.search.yahoo.com/webservices/register_application">Yahoo! Developer Network</a>', '%usage_policy' => '<a href="http://developer.yahoo.com/usagePolicy/index.html">usage policy</a>'))
  );

  return $form;
}

function location_geocode_nl_yahoo($location = array()) {
  $service_url = "http://api.local.yahoo.com/MapsService/V1/geocode?appid=". variable_get('location_geocode_us_yahoo_appid', "YahooDemo") ."&location=";

  $address = location_address2singleline($location);

  $http_reply = drupal_http_request($service_url . urlencode($address));

  // address may have been improperly formatted or invalid
  if ($http_reply->code == 400) {
    return NULL;
  }
  else {
    // got a successful reply, but we only want to return if we have address-level precision
    $matches = array();
    preg_match('/precision="([a-z]*)"/', $http_reply->data, $matches);

    if ($matches[1] != 'address') {
      // The precision we got back was not down to the street-address level
      return NULL;
    }
    else {
      $lat_match = array();
      $lon_match = array();
      $latlon = array();

      if (preg_match('/<Latitude>(.*)<\/Latitude>/', $http_reply->data, $lat_match)) {
        $latlon['lat'] = $lat_match[1];
      }
      else {
        return NULL;
      }


      if (preg_match('/<Longitude>(.*)<\/Longitude>/', $http_reply->data, $lon_match)) {
        $latlon['lon'] = $lon_match[1];

        return $latlon;
      }
      else {
        return NULL;
      }
    }
  }
}

function location_geocode_nl_google($location = array()) {
  $service_url = 'http://maps.google.com/maps/geo?output=xml&key='. variable_get('location_geocode_us_google_apikey', '') .'&q=';

  $address = location_address2singleline($location);

  $http_reply = drupal_http_request($service_url . urlencode($address));

  $status_code_match = array();
  preg_match('/<code>(.*)<\/code>/', $http_reply->data, $status_code_match);
  $status_code = $status_code_match[1];
  if ($status_code != 200) {
    return NULL;
  }

  $accuracy_code_match = array();
  preg_match('/Accuracy="([0-9])"/', $http_reply->data, $accuracy_code_match);
  $accuracy_code = $accuracy_code_match[1];
  //if ($accuracy_code != 8 && $accuracy_code != 7) {
  if($accuracy_code < 4){
    return NULL;
  }

  $latlon_match = array();
  preg_match('/<coordinates>(.*)<\/coordinates>/', $http_reply->data, $latlon_match);

  $latlon_exploded = explode(',', $latlon_match[1]);

  return array('lat' => $latlon_exploded[1], 'lon' => $latlon_exploded[0]);
}

function location_geocode_nl_google_settings(){
  $form = array();

  $form['location_geocode_us_google_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('Google Maps API Key'),
    '#size' => 64,
    '#maxlength' => 128,
    '#default_value' => variable_get('location_geocode_us_google_apikey', ''),
    '#description' => t('In order to use the Google Maps API geocoding web-service, you will need a Google Maps API Key.  You can obtain one at the !sign_up_link for the !google_maps_api.', array('!sign_up_link' => '<a href="http://www.google.com/apis/maps/signup.html">sign-up page</a>', '!google_maps_api' => '<a href="http://www.google.com/apis/maps/">Google Maps API</a>'))
  );

  return $form;
}

/**
 * Parameters:
 *   -> $locationA is an associative array that represents a full location where
 *        'street'       => the street portions of the location
 *        'supplemental' => additional street portion of the location
 *        'province'     => the province, state, or territory
 *        'country'      => lower-cased two-letter ISO code (REQUIRED)
 *   -> $locationB is associative array that represents a full location in the same way that
 *       parameter $locationB does.
 *
 * Returns: a link to driving directions
 *
 * For now, assume site-admin wants American driving directions linked to Yahoo! Driving Directions.
 * Maybe later, we can add some kind of country-specific settings page that allows the site-admin to
 * decide which site to link to for driving directions.
 */
function location_driving_directions_link_nl($locationA, $locationB) {
  return _location_driving_directions_link_nl_yahoo($locationA, $locationB);
}

/**
 * Parameters:
 *    Function that is called by location_driving_directions_link_nl() under assumption that it
 *    is the chosen function
 *
 * Returns:
 *    a URL with HTTP GET variables
 *    Depending on how full the locationes are, the URL will either point to the driving directions
 *    on Yahoo! or, if only partial locationes are provided, a URL that points to the *form* for
 *    Yahoo! driving directions where the form is filled with whatever fields have been provided
 *    for the partial location(es).
 */
function _location_driving_directions_link_nl_yahoo($locationA, $locationB) {
  if (trim($locationB['country']) != 'ca' && trim($locationB['country']) != 'us') {
    return '';
  }

  // These are the fields that need to be in each location if we are to provide a direct
  // link to yahoo directions.  If all of these fields don't have values, then we generate
  // a link to the *form* for Yahoo! driving directions rather than directly to the driving
  // directions themselves.


  foreach ($locationA as $field => $value) {
    $locationA[$field] = trim($value);
  }

  foreach ($locationB as $field => $value) {
    $locationB[$field] = trim($value);
  }

  if (_location_nl_enough_fields_for_yahoo($locationA) && _location_nl_enough_fields_for_yahoo($locationB)) {
    $yahoo_maps_path = 'dd_result';
  }
  else {
    $yahoo_maps_path = 'dd';
  }

  $get_query  = '?';

  $get_query .= 'addr='. urlencode($locationA['street']) .'&amp;';
  $get_query .= 'csz=' . _location_nl_yahoo_csz_get_field($locationA) .'&amp;';
  $get_query .= 'country='. urlencode($locationA['country']) .'&amp;';

  $get_query .= 'taddr='. urlencode($locationB['street']) .'&amp;';
  $get_query .= 'tcsz=' . _location_nl_yahoo_csz_get_field($locationB) .'&amp;';
  $get_query .= 'tcountry='. urlencode($locationB['country']);

  $get_query .= '&amp;getrte='. urlencode('Get Directions');

  return ('http://maps.yahoo.com/'. $yahoo_maps_path . $get_query);
}

function _location_nl_enough_fields_for_yahoo($location) {
  // These are the fields that need to be in each location if we are to provide a direct
  // link to yahoo directions.  If all of these fields don't have values, then we generate
  // a link to the *form* for Yahoo! driving directions rather than directly to the driving
  // directions themselves.
  if (strlen($location['street']) && strlen($location['city']) && strlen($location['province'])) {
    return TRUE;
  }

  if (strlen($location['street']) && strlen($location['postal_code'])) {
    return TRUE;
  }

  if (strlen($location['street']) && strlen($location['city']) && strlen($location['province'])) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Don't mess with this function unless you understand its logic.  It has to do with
 * the question of "to comma or not to comma?"
 */
function _location_nl_yahoo_csz_get_field($location) {
  // For some reasons, to the end of pinpointing a location, Yahoo! Maps and Driving Directions
  // do better a better job with retrieving info based strictly on a Canadian city/province
  // than on a Canadian postal code.
  if ($location['country'] = 'ca') {
    if (strlen($location['city']) && strlen($location['province'])) {
      return urlencode($location['city'] .', '. $location['province']);
    }

    if (strlen($location['postal_code'])) {
      return urlencode($location['postal_code']);
    }
  }
  else {
    if (strlen($location['postal_code'])) {
      return urlencode($location['postal_code']);
    }

    if (strlen($location['city']) && strlen($location['province'])) {
      return urlencode($location['city'] .', '. $location['province']);
    }
  }

  if (strlen($location['city']) || strlen($location['state'])) {
    if (strlen($location['city'])) {
      return urlencode($location['city']);
    }
    else {
      return urlencode($location['state']);
    }
  }
  return '';
}

function _location_nl_geocoder_oneline($location = array()) {
  $line = '';

  $line .= $location['street'] . ', ';

  if (strlen($location['city']) && strlen($location['province']) && strlen($location['postal_code'])) {
    $line .= $location['city'] .', '. $location['province'] .' '. $location['postal_code'];
  }
  elseif (strlen($location['city']) && strlen($location['province'])) {
    $line .= $location['city'] .', '. $location['state'];
  }
  elseif (strlen($location['postal_code'])) {
    if (strlen($location['city']) || strlen($location['state'])) {
      if (strlen($location['city'])) {
        $line .= $location['city'] .', '. $location['postal_code'];
      }
      else {
        $line .= $location['state'] .', '. $location['postal_code'];
      }
    }
    else {
      $line .= $location['postal_code'];
    }
  }
  // DEBUG: commented code is for testing/debugging purposes
  //print '_location_nl_geocoder_oneline() RETURNING '. $line ."<br/>\n";
  return $line;
}
