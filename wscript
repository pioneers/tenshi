#!/usr/bin/env python
# Licensed to Pioneers in Engineering under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  Pioneers in Engineering licenses
# this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License

from __future__ import print_function
import traceback


def recurse(ctx, dirs=None):
    if dirs is None:
        dirs = ['eda', 'controller', 'smartsensor_fw', 'grizzly_firmware',
                'network', 'vm/angelic', 'vm/lua']
    for d in dirs:
        ctx.recurse(d, mandatory=False)


def options(ctx):
    ctx.add_option('--emcc-only',
                   action='store_true',
                   help='Only configure enough (emcc) to build angel-player.')
    recurse(ctx)


def configure(ctx):
    ctx.env['root'] = ctx.path.abspath()
    if ctx.options.emcc_only:
        recurse(ctx, ['network', 'vm/angelic', 'vm/lua'])
    else:
        recurse(ctx)


def build(ctx):
    recurse(ctx)
