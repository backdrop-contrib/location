<?php

/**
 * Returns a lat/lon pair of the approximate center of the given postal code in the given country
 *
 * @param $location
 *   An associative array $location where
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 * @return
 *   An associative array where
 *      'lat' => approximate latitude of the center of the postal code's area
 *      'lon' => approximate longitude of the center of the postal code's area
 *
 */
function location_latlon_rough_us($location = array()) {
  if (!isset($location['postal_code'])) {
    return NULL;
  }

  $result = db_query("SELECT latitude, longitude FROM {zipcodes} WHERE country = '%s' AND zip = '%s'", $location['country'], substr(str_pad($location['postal_code'], 5, '0', STR_PAD_LEFT), 0, 5));
  
  if ($row = db_fetch_object($result)) {
    return array('lat' => $row->latitude, 'lon' => $row->longitude);
  }
  else {
    return NULL;
  }
}

/**
 * Returns a lat/lon pair of the approximate center of the given postal code in the given country
 *
 * @param $location
 *   An associative array $location where only postal code and country are necessary, but can have the keys:
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 * @return
 *   An associative array where
 *      'lat' => approximate latitude of the center of the postal code's area
 *      'lon' => approximate longitude of the center of the postal code's area
 *
 */
function location_get_postalcode_data_us($location = array()) {
  $dash_index == strpos($location['postal_code'], '-');
  // First we strip slash off if we're dealing with a 9-digit US zipcode
  if ($dash_index === FALSE) {
    $location['postal_code'] = substr($location['postal_code'], 0, $dash_index);
  }
  
  // Now we pad the thing and query.
  $res = db_query("SELECT * FROM {zipcodes} where country = '%s' AND zip = '%s'", $location['country'], str_pad($location['postal_code'], 5, "0", STR_PAD_LEFT));
  if ($row = db_fetch_object($res)) {
    return array('lat' => $row->latitude, 'lon' => $row->longitude, 'city' => $row->city, 'province' => $row->state, 'country' => $row->country);
  }
  else {
    return NULL;
  } 
}

/**
 * Parameters:
 *   An associative array $location where
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 *
 */
function location_latlon_exact_us($location = array()) {
  return NULL;
  // By uncommenting the line of code below, you legally acknowledge that you understand that you can only
  // do so under the terms of the Non-Commercial Share-alike license described at http://creativecommons.org/licenses/by-nc-sa/2.0/
  //return _location_latlon_exact_us_geocoder($location);
}


/**
 * Calls up a web-service to retrieve a lat/lon pair for a full, correct U.S. location.
 *
 * @param $location
 *    An associative array that represents an location where
 *        'street'       => is the street location
 *        'supplemental' => any supplemental portion to the street location
 *        'city'         => city name
 *        'province'     => state, province, or territorial abbreviation
 *        'postal_code'  => postal code
 *
 * @return
 *    An associative array where
 *        'lat' => Is a float value in latitude
 *        'lon' => Is a float value in longitude 
 *    If the location supplied does not provide enough information, NULL is returned.
 *    "Enough information" means that there is either 
 *    (a valid 'street' AND valid 'postal_code') OR (valid 'street' AND valid 'city' AND valid 'province')
 */
function _location_latlon_exact_us_geocoder($location = array()) {
  $location_string = '';
  if (isset($location['street']) && trim($location['street']) != '') {
    if (isset($location['postal_code'])) {
      $location_string = $location['street'] .' '. $location['postal_code'];
    }
    elseif (isset($location['city']) && isset($location['province']) && trim($location['city']) != '' && trim($location['province'])) {
      $location_string = $location['street'] .', '. $location['city'] .', '. $location['province'];
    }
    else { // else geocoder.us won't do bidness with you!
      return NULL;
    }
  }
  else {
    return NULL;
  }
  
  $message = new xmlrpcmsg('geocode', array(new xmlrpcval($location_string, 'string')));
  
  $client = new xmlrpc_client('/service/xmlrpc', 'rpc.geocoder.us', 80);
  
  $result = $client->send($message);
  
  //match latitude/longitude and return if present
  $xml_response = $result->serialize();
  $matches = array();
  if (preg_match('/<member><name>lat<\/name>\n<value><double>([^<]+)<\/double><\/value>.*/', $result->serialize(), $matches)) {
    $lat = floatval($matches[1]);
    if (preg_match('/<member><name>long<\/name>\n<value><double>([^<]+)<\/double><\/value>.*/', $result->serialize(), $matches)) {
      return array('lon' => $lat, 'lon' => floatval($matches[1]));
    }
    return NULL;
  }
  return NULL;
}

/**
 * @param $locationA
 *   An associative array that represents a full location where
 *      'street'       => the street portions of the location
 *      'supplemental' => additional street portion of the location
 *      'province'     => the province, state, or territory
 *      'country'      => lower-cased two-letter ISO code (REQUIRED)
 *
 * @param $locationB 
 *   An associative array that represents a full location in the same way that
 *   parameter $locationB does.
 *
 * @return 
 *   A URL with the appropriate HTTP GET params to driving directions
 */
function location_map_link_us($location = array()) {
  $map_links = array();
  // For now, just call the Yahoo function.  May want to make this configurable on some level
  // in order to maintain freedom of choice so users and site-admins don't have to be slaves
  // to Yahoo!.... not that I have anything personal against Yahoo!.
  if ($link = location_map_link_us_yahoo($location)) {
    $map_links['Yahoo! Maps'] = $link;
  }
  
  return $map_links;
}
 

function location_map_link_us_yahoo($location = array()) {
  $get_query = '?';
  if (isset($location['street'])) {
    $get_query .= 'addr='. urlencode($location['street']) .'&amp;';
  }
  if ($location['province'] != '' || $location['city'] != '' || $location['postal_code'] != '') {
    $get_query .= 'csz='. _location_us_yahoo_csz_get_field($location) .'&amp;';
  }
  $get_query .= 'country='. urlencode($location['country']);
  return ('http://maps.yahoo.com/maps_result'. $get_query);
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
function location_driving_directions_link_us($locationA, $locationB) {
  return _location_driving_directions_link_us_yahoo($locationA, $locationB);
}

/**
 * Parameters:
 *    Function that is called by location_driving_directions_link_us() under assumption that it
 *    is the chosen function
 *
 * Returns:
 *    a URL with HTTP GET variables
 *    Depending on how full the locationes are, the URL will either point to the driving directions
 *    on Yahoo! or, if only partial locationes are provided, a URL that points to the *form* for
 *    Yahoo! driving directions where the form is filled with whatever fields have been provided
 *    for the partial location(es).  
 */
function _location_driving_directions_link_us_yahoo($locationA, $locationB) {
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
  
  if (_location_us_enough_fields_for_yahoo($locationA) && _location_us_enough_fields_for_yahoo($locationB)) {
    $yahoo_maps_path = 'dd_result';
  }
  else {
    $yahoo_maps_path = 'dd';
  }
  
  $get_query  = '?';
  
  $get_query .= 'addr='. urlencode($locationA['street']) .'&amp;';
  $get_query .= 'csz=' . _location_us_yahoo_csz_get_field($locationA) .'&amp;';
  $get_query .= 'country='. urlencode($locationA['country']) .'&amp;';
  
  $get_query .= 'taddr='. urlencode($locationB['street']) .'&amp;';
  $get_query .= 'tcsz=' . _location_us_yahoo_csz_get_field($locationB) .'&amp;';
  $get_query .= 'tcountry='. urlencode($locationB['country']);
  
  $get_query .= '&amp;getrte='. urlencode('Get Directions');
  
  return ('http://maps.yahoo.com/'. $yahoo_maps_path . $get_query);
}


function _location_us_enough_fields_for_yahoo($location) {
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

// Don't mess with this function unless you understand its logic.  It has to do with
// the question of "to comma or not to comma?"
function _location_us_yahoo_csz_get_field($location) {
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

function _location_us_geocoder_oneline($location = array()) {
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
  //print '_location_us_geocoder_oneline() RETURNING '. $line ."<br/>\n";
  return $line;
}



/**
 * Returns an associative array of states/territories where
 *   -> the keys are integers starting from 1
 *   -> the values are the English names for those states/territories
 *
 * The states are grouped together at the beginning of the array and sorted
 * alphabetically.
 *
 * The territories are grouped together at the end of the array and sorted
 * alphabetically.
 *
 */
function location_province_list_us() {
  return array('AL' => 'Alabama',
               'AK' => 'Alaska',
               'AZ' => 'Arizona',
               'AR' => 'Arkansas',
               'CA' => 'California',
               'CO' => 'Colorado',
               'CT' => 'Connecticut',
               'DE' => 'Delaware',
               'DC' => 'District Of Columbia',
               'FL' => 'Florida',
               'GA' => 'Georgia',
               'HI' => 'Hawaii',
               'ID' => 'Idaho',
               'IL' => 'Illinois',
               'IN' => 'Indiana',
               'IW' => 'Iowa',
               'KS' => 'Kansas',
               'KY' => 'Kentucky',
               'LA' => 'Louisiana',
               'ME' => 'Maine',
               'MD' => 'Maryland',
               'MA' => 'Massachusetts',
               'MI' => 'Michigan',
               'MN' => 'Minnesota',
               'MS' => 'Mississippi',
               'MO' => 'Missouri',
               'MT' => 'Montana',
               'NE' => 'Nebraska',
               'NV' => 'Nevada',
               'NH' => 'New Hampshire',
               'NJ' => 'New Jersey',
               'NM' => 'New Mexico',
               'NY' => 'New York',
               'NC' => 'North Carolina',
               'ND' => 'North Dakota',
               'OH' => 'Ohio',
               'OK' => 'Oklahoma',
               'OR' => 'Oregon',
               'PA' => 'Pennsylvania',
               'RI' => 'Rhode Island',
               'SC' => 'South Carolina',
               'SD' => 'South Dakota',
               'TN' => 'Tennessee',
               'TX' => 'Texas',
               'UT' => 'Utah',
               'VT' => 'Vermont',
               'VA' => 'Virginia',
               'WA' => 'Washington',
               'WV' => 'West Virginia',
               'WI' => 'Wisconsin',
               'WY' => 'Wyoming',
               
               'AS' => 'American Samoa',
               'FM' => 'Federated States of Micronesia',
               'GU' => 'Guam',
               'MH' => 'Marshall Islands',
               'MP' => 'Northern Mariana Islands',
               'PW' => 'Palau',
               'PR' => 'Puerto Rico',
               'VI' => 'Virgin Islands'
              );
}


/**
 * Returns an associative array of states/territories where
 *   -> the keys are integers starting from 1
 *   -> the values are the English names for those states/territories
 *
 * The states are grouped together at the beginning of the array and sorted
 * alphabetically.
 *
 * The territories are grouped together at the end of the array and sorted
 * alphabetically.
 *
 * Currently not being used, but may be in order to be compatible with CiviCRM
 * TODO: These numeric indices need to be changed to match those in the CiviCRM database
 */
function location_province_list_numeric_us() {
  return array('001' => 'Alabama',
               '002' => 'Alaska',
               '003' => 'Arizona',
               '004' => 'Arkansas',
               '005' => 'California',
               '006' => 'Colorado',
               '007' => 'Connecticut',
               '008' => 'Delaware',
               '009' => 'District Of Columbia',
               '010' => 'Florida',
               '011' => 'Georgia',
               '012' => 'Hawaii',
               '013' => 'Idaho',
               '014' => 'Illinois',
               '015' => 'Indiana',
               '016' => 'Iowa',
               '017' => 'Kansas',
               '018' => 'Kentucky',
               '019' => 'Louisiana',
               '020' => 'Maine',
               '021' => 'Maryland',
               '022' => 'Massachusetts',
               '023' => 'Michigan',
               '024' => 'Minnesota',
               '025' => 'Mississippi',
               '026' => 'Missouri',
               '027' => 'Montana',
               '028' => 'Nebraska',
               '029' => 'Nevada',
               '030' => 'New Hampshire',
               '031' => 'New Jersey',
               '032' => 'New Mexico',
               '033' => 'New York',
               '034' => 'North Carolina',
               '035' => 'North Dakota',
               '036' => 'Ohio',
               '037' => 'Oklahoma',
               '038' => 'Oregon',
               '039' => 'Pennsylvania',
               '040' => 'Rhode Island',
               '041' => 'South Carolina',
               '042' => 'South Dakota',
               '043' => 'Tennessee',
               '044' => 'Texas',
               '045' => 'Utah',
               '046' => 'Vermont',
               '047' => 'Virginia',
               '048' => 'Washington',
               '049' => 'West Virginia',
               '050' => 'Wisconsin',
               '051' => 'Wyoming',
               
               '052' => 'American Samoa',
               '053' => 'Federated States of Micronesia',
               '054' => 'Guam',
               '055' => 'Marshall Islands',
               '056' => 'Northern Mariana Islands',
               '057' => 'Palau',
               '058' => 'Puerto Rico',
               '059' => 'Virgin Islands'
              );
}

?>
