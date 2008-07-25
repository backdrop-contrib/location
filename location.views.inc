<?php
// $Id$

/**
 * @file
 * Views 2 support for Location.
 */

/**
 * Implementatin of hook_views_data().
 */
function location_views_data() {
  // ----------------------------------------------------------------
  // location table -- basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.

  $data['location']['table']['group'] = t('Location');

  // Advertise this table as a possible base table
  $data['location']['table']['base'] = array(
    'field' => 'lid',
    'title' => t('Location'),
    'help' => t("Locations are addresses and map coordinates."),
    'weight' => -10,
  );

  $data['location']['table']['join'] = array(
    'location_instance' => array(
      'left_table' => 'location_instance',
      'left_field' => 'lid',
      'field' => 'lid',
    ),
    'node' => array(
      'table' => 'location_instance',
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'node_revisions' => array(
      'table' => 'location_instance',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      'table' => 'location_instance',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  // ----------------------------------------------------------------
  // location table -- fields

  // lid
  $data['location']['lid'] = array(
    'title' => t('Lid'),
    'help' => t('The location ID of the location.'), // The help that appears on the UI,
    // Information for displaying the lid
    'field' => array(
      'handler' => 'views_handler_field', // @@@
      'click sortable' => TRUE,
    ),
    // Information for accepting a lid as an argument
    /*
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'nid',
    ),
    */
    // Information for accepting a lid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // Information for sorting on a lid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['location']['name'] = array(
    'title' => t('Name'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['street'] = array(
    'title' => t('Street'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['additional'] = array(
    'title' => t('Additional'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['city'] = array(
    'title' => t('City'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );


  $data['location']['postal_code'] = array(
    'title' => t('Postal Code'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );




  $data['location_instance']['table']['group']  = t('Location');

  // @@@ Is all this actually needed?
  $data['location_instance']['table']['join'] = array(
    'location' => array(
      'left_field' => 'lid',
      'field' => 'lid',
    ),
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  return $data;
}
