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

CSL.Node.number = {
	build: function (state, target) {
		var func;
		CSL.Util.substituteStart.call(this, state, target);
		//
		// This should push a rangeable object to the queue.
		//
		if (this.strings.form === "roman") {
			this.formatter = state.fun.romanizer;
		} else if (this.strings.form === "ordinal") {
			this.formatter = state.fun.ordinalizer;
		} else if (this.strings.form === "long-ordinal") {
			this.formatter = state.fun.long_ordinalizer;
		}
		if ("undefined" === typeof this.successor_prefix) {
			this.successor_prefix = state[state.tmp.area].opt.layout_delimiter;
		}
		if ("undefined" === typeof this.splice_prefix) {
			this.splice_prefix = state[state.tmp.area].opt.layout_delimiter;
		}
		// is this needed?
		//if ("undefined" === typeof this.splice_prefix){
		//	this.splice_prefix = state[state.tmp.area].opt.layout_delimiter;
		//}
		//
		// Whether we actually stick a number object on
		// the output queue depends on whether the field
		// contains a pure number.
		//
		// push number or text
		func = function (state, Item) {
			var varname, num, number, m, j, jlen;
			varname = this.variables[0];
			state.parallel.StartVariable(this.variables[0]);
			state.parallel.AppendToVariable(Item[this.variables[0]]);

			if (varname === "page-range" || varname === "page-first") {
				varname = "page";
			}
			num = Item[varname];
			if ("undefined" !== typeof num) {
				if (this.variables[0] === "page-first") {
					m = num.split(/\s*(?:&|,|-)\s*/);
					num = m[0];
				}
				// Check for valid multiple items.  If found, 
				// open a new empty level and iterate this, with 
				// intervening punctuation set as prefixes to the 
				// second and succeeding output blobs.
				//
				// (If we are in a cs:sort node, we use
				// the first number encountered only)
				var prefixes = num.split(/[0-9]+/);
				var all_with_spaces = true;
				// all_with_spaces will be false only if it is
				//   of length >= 3, and has a string with no
				//   space between position 1 and position length - 2.
				// Single numbers will evaluate true, because
				//   the length of prefixes in that case will be 2.
				for (var i = 1, ilen = prefixes.length - 1; i < ilen; i += 1) {
				    if (prefixes[i].indexOf(" ") === -1) {
						all_with_spaces = false;
						break;
				    }
				}
				if (state.tmp.area !== "citation_sort"
				  && state.tmp.area !== "bibliography_sort"
				  && all_with_spaces 
				  && !num.match(/[^- 0-9,&]/)) {
					// For endash.  Wait for complaints on this one.
					//num = num.replace("-","\u2013", "g");
					var nums = num.match(/[0-9]+/g);
					// Expand ranges
					var range_ok = true;
					for (i = prefixes.length - 2; i > 0; i += -1) {
						if (prefixes && prefixes[i].indexOf("-") > -1) {
							var start = parseInt(nums[i - 1], 10);
							var end = parseInt(nums[i], 10);
							// Block silly values
							if (start >= end || start < (end - 1000)) {
								range_ok = false;
								break;
							}
							var replacement = [];
							for (j = start, jlen = end + 1; j < jlen; j += 1) {
								replacement.push(""+j);
							}
								nums = nums.slice(0, i - 1).concat(replacement).concat(nums.slice(i + 1));
						}
					}
					if (range_ok) {
						nums = nums.sort(function (a,b) {
							a = parseInt(a, 10);
							b = parseInt(b, 10);
							if (a > b) {
								return 1;
							} else if (a < b) {
								return -1;
							} else {
								return 0;
							}
						});
						// Eliminate duplicate numbers
						for (i = nums.length; i > -1; i += -1) {
							if (nums[i] === nums[i + 1]) {
								nums = nums.slice(0, i).concat(nums.slice(i + 1));
							}
						}
						state.output.openLevel("empty");
						for (i = 0, ilen = nums.length; i < ilen; i += 1) {
							num = parseInt(nums[i], 10);
							number = new CSL.NumericBlob(num, this);
							number.gender = state.opt["noun-genders"][varname];
							if (i > 0) {
								// state.output.append(prefixes[i], "empty");
								number.successor_prefix = " & ";
								number.range_prefix = "-";
								number.splice_prefix = ", ";
							}
							state.output.append(number, "literal");
						}
						state.output.closeLevel("empty");
					} else {
						state.output.append(num, this);
				    }
				} else if (!all_with_spaces || prefixes.length > 2) {
				    // Don't attempt to apply numeric formatting
				    // or to normalize the content for weird
				    // numbers like "1-505" (no space between the
				    // numbers and the hyphen suggests that 505 might
				    // be meant as a leaf number)
				    state.output.append(num, this);
				} else {
					// Single number
					m = num.match(/\s*([0-9]+)(?:[^-]* |[^-]*$)/);
					if (m) {
						num = parseInt(m[1], 10);
						number = new CSL.NumericBlob(num, this);
						number.gender = state.opt["noun-genders"][varname];
						state.output.append(number, "literal");
				    } else {
						state.output.append(num, this);
				    }
				}
			}
			state.parallel.CloseVariable("number");
		};
		this.execs.push(func);

		target.push(this);
		CSL.Util.substituteEnd.call(this, state, target);
	}
};


