<?php
// $Id$

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
*/
function location_map_link_de($location = array()) {
  $map_links = array();
  // For now, just call the tinfo function.  May want to make this configurable on some level
  // in order to maintain freedom of choice so users and site-admins don't have to be slaves
  // to tinfo!.... not that I have anything personal against tinfo!.
  if ($link = _location_map_link_de_tinfo($location)) {
    $map_links['T-info'] = $link;
  }
  
  return $map_links;
}


function _location_map_link_de_tinfo($location = array()) {
  $get_query = '?';
  $get_query .= 'map_input.country=2';
  if (isset($location['street'])) {
    $get_query .= '&amp;' . 'map_input.street='. urlencode($location['street']);
  }
  if ($location['postal_code'] != '') {
    $get_query .= '&amp;' . 'map_input.zip='. urlencode($location['postal_code']);
  }
  if ($location['city'] != '') {
    $get_query .= '&amp;' . 'map_input.city_manual='. urlencode($location['city']);
  }
//  if ($location['number'] != '') {
//    $get_query .= '&amp;' . 'map_input.number='. urlencode($location['number']);
//  }
  return ('http://www.t-info.de/map/jumppage/map.jsp'. $get_query);
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
* For now, assume site-admin wants driving directions linked to tinfo!
* Maybe later, we can add some kind of country-specific settings page that allows the site-admin to
* decide which site to link to for driving directions.
*/
function location_driving_directions_link_de($locationA, $locationB) {
  return _location_driving_directions_link_de_tinfo($locationA, $locationB);
}


/**
* Parameters:
*    Function that is called by location_driving_directions_link_ca() under assumption that it
*    is the chosen function
*
* Returns:
*    a URL with HTTP GET variables
*    Depending on how full the locationes are, the URL will either point to the driving directions
*    on tinfo! or, if only partial locationes are provided, a URL that points to the *form* for
*    tinfo! driving directions where the form is filled with whatever fields have been provided
*    for the partial location(es).  
*/
function _location_driving_directions_link_de_tinfo($locationA, $locationB) {
  
  foreach ($locationA as $field => $value) {
    $locationA[$field] = trim($value);
  }
  
  foreach ($locationB as $field => $value) {
    $locationB[$field] = trim($value);
  }
  
  if ($locationA['country'] == 'de' && $locationB['country'] == 'de') {
  
    $get_query = '?';
    $get_query .= 'route_input.start_country=2';
    if (isset($location['street'])) {
      $get_query .= '&amp;' . 'route_input.start_street='. urlencode($location['street']);
    }
    if ($location['postal_code'] != '') {
      $get_query .= '&amp;' . 'route_input.start_zip='. urlencode($location['postal_code']);
    }
    if ($location['city'] != '') {
      $get_query .= '&amp;' . 'route_input.start_city_manual='. urlencode($location['city']);
    }
    $get_query .= 'route_input.target_country=2';
    if (isset($location['street'])) {
      $get_query .= '&amp;' . 'route_input.target_street='. urlencode($location['street']);
    }
    if ($location['postal_code'] != '') {
      $get_query .= '&amp;' . 'route_input.target_zip='. urlencode($location['postal_code']);
    }
    if ($location['city'] != '') {
      $get_query .= '&amp;' . 'route_input.target_city_manual='. urlencode($location['city']);
    }
    return ('http://www.t-info.de/map/jumppage/route_stations.jsp'. $get_query);
  }
}

function theme_location_de($location = array()) {
  $output = '';
  if (count($location)) {
    $output .= "\n";
    $output .= '<dl class="location">'."\n";
    
    if (isset($location['street'])) {
      $output .= '<dd>'. $location['street'] .'</dd>';
    }
    
    if (isset($location['additional'])) {
      $output .= '<dd>' . $location['additional'] . '</dd>';
    }
    
    $postal_isset = (isset($location['postal_code']) && strlen($location['postal_code']));
    $city_isset = (isset($location['city']) && strlen($location['postal_code']));
    if ($postal_isset || $city_isset) {
      $output .= '<dd>';
      if ($postal_isset) {
        $output .= $location['postal_code'];
      }
      if ($postal_isset && $city_isset) {
        $output .= ' ';
      }
      if ($city_isset) {
        $output .= $location['city'];
      }
      $output .= "</dd>\n";
    }
    
    $output .= '<dd>'. t('Germany') ."</dd>\n";
  }
  return $output;
}

?>
