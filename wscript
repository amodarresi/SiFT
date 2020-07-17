## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('sift', ['internet', 'wifi', 'mesh', 'applications'])
    module.includes = '.'
    module.source = [
        'model/sift-geo.cc',
        'model/sift-routing.cc',
        'model/sift-header.cc',
        'helper/sift-helper.cc',
        'helper/sift-main-helper.cc',
        ]
        
    module_test = bld.create_ns3_module_test_library('sift')
    module_test.source = [
        'test/sift-test-suite.cc',
        ]
        
    headers = bld(features='ns3header')
    headers.module = 'sift'
    headers.source = [
        'model/sift-geo.h',
        'model/sift-routing.h',
        'model/sift-header.h',
        'model/sift-repository.h',
        'helper/sift-helper.h',
        'helper/sift-main-helper.h',
        ]

    if (bld.env['ENABLE_EXAMPLES']):
      bld.recurse('examples')

    bld.ns3_python_bindings()
