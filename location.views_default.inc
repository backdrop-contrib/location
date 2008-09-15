<?php
// $Id$

/**
 * @file
 * Default views for location module.
 */

/**
 * Implementation of hook_default_view_views().
 */
function location_views_default_views() {
  $view = new view;
  $view->name = 'location_table';
  $view->description = 'Node location table.';
  $view->tag = '';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = TRUE;
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'title' => array(
      'label' => 'Title:',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'street' => array(
      'label' => 'Street:',
      'exclude' => 0,
      'id' => 'street',
      'table' => 'location',
      'field' => 'street',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'city' => array(
      'label' => 'City:',
      'exclude' => 0,
      'id' => 'city',
      'table' => 'location',
      'field' => 'city',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'province' => array(
      'label' => 'Province:',
      'style' => 'name',
      'exclude' => 0,
      'id' => 'province',
      'table' => 'location',
      'field' => 'province',
      'relationship' => 'none',
    ),
    'postal_code' => array(
      'label' => 'Postal Code:',
      'exclude' => 0,
      'id' => 'postal_code',
      'table' => 'location',
      'field' => 'postal_code',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'country' => array(
      'label' => 'Country:',
      'style' => 'name',
      'exclude' => 0,
      'id' => 'country',
      'table' => 'location',
      'field' => 'country',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status_extra' => array(
      'operator' => '=',
      'value' => '',
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status_extra',
      'table' => 'node',
      'field' => 'status_extra',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'none',
  ));
  $handler->override_option('style_plugin', 'table');
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('path', 'location/views');
  $handler->override_option('menu', array(
    'type' => 'normal',
    'title' => 'Location table',
    'weight' => '0',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'weight' => 0,
  ));
  $views[$view->name] = $view;


  return $views;
}
