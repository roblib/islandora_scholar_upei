/*
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights
 * Reserved.
 *
 * The contents of this file are subject to the Common Public
 * Attribution License Version 1.0 (the “License”); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at:
 *
 * http://bitbucket.org/fbennett/citeproc-js/src/tip/LICENSE.
 *
 * The License is based on the Mozilla Public License Version 1.1 but
 * Sections 14 and 15 have been added to cover use of software over a
 * computer network and provide for limited attribution for the
 * Original Developer. In addition, Exhibit A has been modified to be
 * consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an “AS IS”
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the citation formatting software known as
 * "citeproc-js" (an implementation of the Citation Style Language
 * [CSL]), including the original test fixtures and software located
 * under the ./std subdirectory of the distribution archive.
 *
 * The Original Developer is not the Initial Developer and is
 * __________. If left blank, the Original Developer is the Initial
 * Developer.
 *
 * The Initial Developer of the Original Code is Frank G. Bennett,
 * Jr. All portions of the code written by Frank G. Bennett, Jr. are
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Affero General Public License (the [AGPLv3]
 * License), in which case the provisions of [AGPLv3] License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the [AGPLv3] License
 * and not to allow others to use your version of this file under the
 * CPAL, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the
 * [AGPLv3] License. If you do not delete the provisions above, a
 * recipient may use your version of this file under either the CPAL
 * or the [AGPLv3] License.”
 */

CSL.Util.PageRangeMangler = {};

CSL.Util.PageRangeMangler.getFunction = function (state) {
	var rangerex, pos, len, stringify, listify, expand, minimize, minimize_internal, chicago, lst, m, b, e, ret, begin, end, ret_func, ppos, llen;
	rangerex = /([a-zA-Z]*)([0-9]+)\s*-\s*([a-zA-Z]*)([0-9]+)/;

	stringify = function (lst) {
		len = lst.length;
		for (pos = 1; pos < len; pos += 2) {
			if ("object" === typeof lst[pos]) {
				lst[pos] = lst[pos].join("");
			}
		}
		return lst.join("");
	};

	listify = function (str) {
		var m, lst, ret;
		// Workaround for Internet Explorer
		m = str.match(/([a-zA-Z]*[0-9]+\s*-\s*[a-zA-Z]*[0-9]+)/g);
		lst = str.split(/[a-zA-Z]*[0-9]+\s*-\s*[a-zA-Z]*[0-9]+/);

		if (lst.length === 0) {
			ret = m;
		} else {
			ret = [lst[0]];
			for (pos = 1, len = lst.length; pos < len; pos += 1) {
				ret.push(m[pos - 1]);
				ret.push(lst[pos]);
			}
		}
		return ret;
	};

	expand = function (str) {
		str = "" + str;
		lst = listify(str);
		len = lst.length;
		for (pos = 1; pos < len; pos += 2) {
			m = lst[pos].match(rangerex);
			if (m) {
				if (!m[3] || m[1] === m[3]) {
					if (m[4].length < m[2].length) {
						m[4] = m[2].slice(0, (m[2].length - m[4].length)) + m[4];
					}
					if (parseInt(m[2], 10) < parseInt(m[4], 10)) {
						m[3] = "\u2013" + m[1];
						lst[pos] = m.slice(1);
					}
				}
			}
			if ("string" === typeof lst[pos]) {
				lst[pos] = lst[pos].replace("-", "\u2013");
			}
		}
		return lst;
	};

	minimize = function (lst) {
		len = lst.length;
		for (pos = 1; pos < len; pos += 2) {
			lst[pos][3] = minimize_internal(lst[pos][1], lst[pos][3]);
			if (lst[pos][2].slice(1) === lst[pos][0]) {
				lst[pos][2] = "\u2013";
			}
		}
		return stringify(lst);
	};

	minimize_internal = function (begin, end) {
		b = ("" + begin).split("");
		e = ("" + end).split("");
		ret = e.slice();
		ret.reverse();
		if (b.length === e.length) {
			llen = b.length;
			for (ppos = 0; ppos < llen; ppos += 1) {
				if (b[ppos] === e[ppos]) {
					ret.pop();
				} else {
					break;
				}
			}
		}
		ret.reverse();
		return ret.join("");
	};

	chicago = function (lst) {
		len = lst.length;
		for (pos = 1; pos < len; pos += 2) {
			if ("object" === typeof lst[pos]) {
				m = lst[pos];
				begin = parseInt(m[1], 10);
				end = parseInt(m[3], 10);
				if (begin > 100 && begin % 100 && parseInt((begin / 100), 10) === parseInt((end / 100), 10)) {
					m[3] = "" + (end % 100);
				} else if (begin >= 10000) {
					m[3] = "" + (end % 1000);
				}
			}
			if (m[2].slice(1) === m[0]) {
				m[2] = "\u2013";
			}
		}
		return stringify(lst);
	};

	//
	// The top-level option handlers.
	//
	if (!state.opt["page-range-format"]) {
		ret_func = function (str) {
			return str;
		};
	} else if (state.opt["page-range-format"] === "expanded") {
		ret_func = function (str) {
			var lst = expand(str);
			return stringify(lst);
		};
	} else if (state.opt["page-range-format"] === "minimal") {
		ret_func = function (str) {
			var lst = expand(str);
			return minimize(lst);
		};
	} else if (state.opt["page-range-format"] === "chicago") {
		ret_func = function (str) {
			var lst = expand(str);
			return chicago(lst);
		};
	}

	return ret_func;
};

