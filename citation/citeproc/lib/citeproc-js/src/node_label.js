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

CSL.Node.label = {
	build: function (state, target) {
		var func, term, plural, form, debug;
		debug = false;
		if (state.build.name_flag) {
			this.strings.label_position = CSL.AFTER;
		} else {
			this.strings.label_position = CSL.BEFORE;
		}
		// set label info
		func = function (state, Item) {
			state.output.addToken("label", false, this);
		};
		this.execs.push(func);
		if (state.build.term) {
			term = state.build.term;
			plural = false;
			if (!this.strings.form) {
				this.strings.form = "long";
			}
			func = function (state, Item, item) {
				// This is abstracted away, because the same
				// logic must be run in cs:names.
				var termtxt = CSL.evaluateLabel(this, state, Item, item, term);
				state.output.append(termtxt, this);
			};
			this.execs.push(func);
			state.build.plural = false;
			state.build.term = false;
			state.build.form = false;
		}
		target.push(this);
	}
};

CSL.evaluateLabel = function (node, state, Item, item, term, termvar) {
	var myterm;
	if ("locator" === term) {
		if (item && item.label) {
			myterm = item.label;
		}
		if (!myterm) {
			myterm = "page";
		}
	} else {
		myterm = term;
	}
	if (!termvar) {
		termvar = term;
	}
	// Plurals detection.
	var plural = node.strings.plural;
	if ("number" !== typeof plural) {
		if (CSL.CREATORS.indexOf(termvar) > -1) {
			// check for plural creator
			// This is a little tricky, because an institutional
			// name following an individual is an affiliation.
			var creatorCount = -1;
			var lastWasPerson = true;
			plural = 0;
			for (var i = 0, ilen = Item[termvar].length; i < ilen; i += 1) {
				if (Item[termvar][i].given) {
					creatorCount += 1;
					lastWasPerson = true;
				} else {
					if (!lastWasPerson) {
						creatorCount += 1;
					}
					lastWasPerson = false;
				}
				if (creatorCount) {
					plural = 1;
					break;
				}
			}
		} else if ("locator" == term) {
			// check for plural flat field in supplementary item
			if (item) {
				plural = CSL.evaluateStringPluralism(item.locator);				
			}
		} else if (Item[term]) {
			// check for plural flat field in main Item
			plural = CSL.evaluateStringPluralism(Item[term]);			
		}
		// cleanup
		if ("number" !== typeof plural) {
			plural = 0;
		}
	}
	var termtxt = state.getTerm(myterm, node.strings.form, plural);
	if (node.strings["strip-periods"]) {
		termtxt = termtxt.replace(/\./g, "");
	}
	return termtxt;
}

CSL.evaluateStringPluralism = function (str) {
	if (str && str.match(/(?:[0-9], *[0-9]| and |&|[0-9] *- *[0-9])/)) {
		return 1;
	} else {
		return 0;
	}
};
