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

//
// XXXXX: note to self, the parallels machinery should be completely
// disabled when sorting of citations is requested.
//
//
// XXXXX: also mark the entry as "parallel" on the citation
// object.
//
//
// XXXXX: thinking forward a bit, we're going to need a means
// of snooping and mangling delimiters.  Inter-cite delimiters
// can be easily applied; it's just a matter of adjusting
// this.tmp.splice_delimiter (?) on the list of attribute
// bundles after a cite or set of cites is completed.
// That happens in cmd_cite.js.  We also need to do two
// things: (1) assure that volume, number, journal and
// page are contiguous within the cite, with no intervening
// rendered variables [done]; and (2) strip affixes to the series,
// so that the sole splice string is the delimiter.  This
// latter will need a walk of the output tree, but it's
// doable.
//
// The advantage of doing things this way is that
// the parallels machinery is encapsulated in a set of
// separate functions that do not interact with cite
// composition.
//

/**
 * Initializes the parallel cite tracking arrays
 */
CSL.Parallel = function (state) {
	this.state = state;
	this.sets = new CSL.Stack([]);
	this.try_cite = true;
	this.use_parallels = true;
};

CSL.Parallel.prototype.isMid = function (variable) {
	return ["names", "section", "volume", "container-title", "issue", "page", "locator"].indexOf(variable) > -1;
};

CSL.Parallel.prototype.StartCitation = function (sortedItems, out) {
	if (this.use_parallels) {
		this.sortedItems = sortedItems;
		this.sortedItemsPos = -1;
		this.sets.clear();
		this.sets.push([]);
		this.in_series = true;
		this.delim_counter = 0;
		this.delim_pointers = [];
		if (out) {
			this.out = out;
		} else {
			this.out = this.state.output.queue;
		}

	}
};

/**
 * Sets up an empty variables tracking object.
 *
 */
CSL.Parallel.prototype.StartCite = function (Item, item, prevItemID) {
	var position, len, pos, x, curr, master, last_id, prev_locator, curr_locator, is_master, parallel;
	if (this.use_parallels) {
		// print("StartCite");
		if (this.sets.value().length && this.sets.value()[0].itemId == Item.id) {
			this.ComposeSet();
		}
		this.sortedItemsPos += 1;
		if (item) {
			position = item.position;
		}
		//
		// Parallel items are tracked in the registry
		// against each reference item, on first references
		// only.  The parallel value is the ID of the reference
		// item first in the list of parallels, otherwise it
		// is false.
		//
		this.try_cite = true;
		len = CSL.PARALLEL_MATCH_VARS.length;
		for (pos = 0; pos < len; pos += 1) {
			x = CSL.PARALLEL_MATCH_VARS[pos];
			if (!Item[x] || CSL.PARALLEL_TYPES.indexOf(Item.type) === -1) {
				this.try_cite = false;
				if (this.in_series) {
					// clean list is pushed to stack later.  this.sets.push([]);
					this.in_series = false;
				}
				break;
			}
		}
		this.cite = {};
		this.cite.front = [];
		this.cite.mid = [];
		this.cite.back = [];
		this.cite.front_collapse = {};
		this.cite.back_forceme = [];
		this.cite.position = position;
		this.cite.Item = Item;
		this.cite.itemId = "" + Item.id;
		this.cite.prevItemID = "" + prevItemID;
		this.target = "front";
		//
		// Reevaluate position of this cite, if it follows another, in case it
		// is a lurking ibid reference.
		//
		if (this.sortedItems && this.sortedItemsPos > 0 && this.sortedItemsPos < this.sortedItems.length) {
			// This works, and I am absolutely certain that I have
			// no idea how or why.
			curr = this.sortedItems[this.sortedItemsPos][1];
			last_id = "" + this.sortedItems[(this.sortedItemsPos - 1)][1].id;
			master = this.state.registry.registry[last_id].parallel;
			prev_locator = false;
			if (master == curr.id) {
				len = this.sortedItemsPos - 1;
				for (pos = len; pos > -1; pos += -1) {
					if (this.sortedItems[pos][1].id == Item.id) {
						prev_locator = this.sortedItems[pos][1].locator;
						break;
					}
				}
				curr_locator = this.sortedItems[this.sortedItemsPos][1].locator;
				if (!prev_locator && curr_locator) {
					curr.position = CSL.POSITION_IBID_WITH_LOCATOR;
				} else if (curr_locator === prev_locator) {
					curr.position = CSL.POSITION_IBID;
					//**print("setting IBID in util_parallel");
					//**print(" === "+this.sets.value().length);
				} else {
					curr.position = CSL.POSITION_IBID_WITH_LOCATOR;
				}
			}
		}
		this.force_collapse = false;
		if (this.state.registry.registry[Item.id].parallel) {
			this.force_collapse = true;
		}
	}
};

/**
 * Initializes scratch object and variable name string
 * for tracking a single variable.
 */
CSL.Parallel.prototype.StartVariable = function (variable) {
	if (this.use_parallels && (this.try_cite || this.force_collapse)) {
		this.variable = variable;
		this.data = {};
		this.data.value = "";
		this.data.blobs = [];
		var is_mid = this.isMid(variable);
		if (this.target === "front" && is_mid) {
			this.target = "mid";
		} else if (this.target === "mid" && !is_mid && this.cite.Item.title) {
			this.target = "back";
		} else if (this.target === "back" && is_mid) {
			this.try_cite = true;
			this.in_series = false;
		}
		// Exception for docket number.  Necessary for some
		// civil law cites (France), which put the docket number
		// at the end of the first of a series of references.
		if (variable === "number") {
			this.cite.front.push(variable);
		} else if (CSL.PARALLEL_COLLAPSING_MID_VARSET.indexOf(variable) > -1) {
			this.cite.front.push(variable);
		} else {
			this.cite[this.target].push(variable);
		}
	}
};

/**
 * Adds a blob to the the scratch object.  Invoked through
 * state.output.append().  The pointer is used to snip
 * the target blob out of the output queue if appropriate,
 * after parallels detection is complete.
 */
CSL.Parallel.prototype.AppendBlobPointer = function (blob) {
	if (this.use_parallels && this.variable && (this.try_cite || this.force_collapse) && blob && blob.blobs) {
		this.data.blobs.push([blob, blob.blobs.length]);
	}
};

/**
 * Adds string data to the current variable
 * in the variables tracking object.
 */
CSL.Parallel.prototype.AppendToVariable = function (str, varname) {
	if (this.use_parallels && (this.try_cite || this.force_collapse)) {
			// ZZZZZ
		if (this.target !== "back" || true) {
			//print("  setting: "+str);
			this.data.value += "::" + str;
		} else {
			var prev = this.sets.value()[(this.sets.value().length - 1)];
			if (prev) {
				if (prev[this.variable]) {
					if (prev[this.variable].value) {
						//**print("append var "+this.variable+" as value "+this.data.value);
						this.data.value += "::" + str;
					}
				}
			}
		}
	}
};

/**
 * Merges scratch object to the current cite object.
 * Checks variable content, and possibly deletes the
 * variables tracking object to abandon parallel cite processing
 * for this cite.  [??? careful with the logic here, current
 * item can't necessarily be discarded; it might be the first
 * member of an upcoming sequence ???]
 */
CSL.Parallel.prototype.CloseVariable = function (hello) {
	if (this.use_parallels && (this.try_cite || this.force_collapse)) {
		this.cite[this.variable] = this.data;
		if (this.sets.value().length > 0) {
			var prev = this.sets.value()[(this.sets.value().length - 1)];
			if (this.target === "front") {
				if (!(!prev[this.variable] && !this.data.value) && (!prev[this.variable] || this.data.value !== prev[this.variable].value)) {
					// evaluation takes place later, at close of cite.
					//this.try_cite = true;
					this.in_series = false;
				}
			} else if (this.target === "mid") {
				if (CSL.PARALLEL_COLLAPSING_MID_VARSET.indexOf(this.variable) > -1) {
					if (prev[this.variable]) {
						if (prev[this.variable].value === this.data.value) {
							this.cite.front_collapse[this.variable] = true;
						} else {
							this.cite.front_collapse[this.variable] = false;
						}
					} else {
						this.cite.front_collapse[this.variable] = false;
					}
				}
			} else if (this.target === "back") {
				if (prev[this.variable]) {
					if (this.data.value !== prev[this.variable].value && this.sets.value().slice(-1)[0].back_forceme.indexOf(this.variable) === -1) {
						//print(this.variable);
						//print(this.sets.value().slice(-1)[0].back_forceme);
						// evaluation takes place later, at close of cite.
						//this.try_cite = true;
						//**print("-------------- reset --------------");
						//print("  breaking series");
						this.in_series = false;
					}
				}
			}
		}
	}
	this.variable = false;
};

/**
 * Merges current cite object to the
 * tracking array, and evaluate maybe.
 */
CSL.Parallel.prototype.CloseCite = function () {
	var x, pos, len, has_issued, use_journal_info, volume_pos, container_title_pos, section_pos;
	if (this.use_parallels) {
		use_journal_info = false;
		if (!this.cite.front_collapse["container-title"]) {
			use_journal_info = true;
		}
		if (this.cite.front_collapse.volume === false) {
			use_journal_info = true;
		}
		if (this.cite.front_collapse.section === false) {
			use_journal_info = true;
		}
		if (use_journal_info) {
			this.cite.use_journal_info = true;
			section_pos = this.cite.front.indexOf("section");
			if (section_pos > -1) {
				this.cite.front = this.cite.front.slice(0,section_pos).concat(this.cite.front.slice(section_pos + 1));
			}
			volume_pos = this.cite.front.indexOf("volume");
			if (volume_pos > -1) {
				this.cite.front = this.cite.front.slice(0,volume_pos).concat(this.cite.front.slice(volume_pos + 1));
			}
			container_title_pos = this.cite.front.indexOf("container-title");
			if (container_title_pos > -1) {
				this.cite.front = this.cite.front.slice(0,container_title_pos).concat(this.cite.front.slice(container_title_pos + 1));
			}
		}
		if (!this.in_series && !this.force_collapse) {
			this.ComposeSet(true);
		}
		//**print("[pushing cite]");
		if (this.sets.value().length === 0) {
			has_issued = false;
			for (pos = 0, len = this.cite.back.length; pos < len; pos += 1) {
				x = this.cite.back[pos];
				//**print("  ->issued="+this.cite.issued);
				//for (var x in this.cite.issued) {
				//	print("..."+x);
				//}
				if (x === "issued" && this.cite.issued && this.cite.issued.value) {
					//print("HAS ISSUED");
					has_issued = true;
					break;
				}
			}
			if (!has_issued) {
				//print("  setting issued in back_forceme variable culling list");
				this.cite.back_forceme.push("issued");
			}
		} else {
			//print("  renewing");
			this.cite.back_forceme = this.sets.value().slice(-1)[0].back_forceme;
		}
		//print("WooHoo lengtsh fo sets value list: "+this.sets.mystack.length);
		this.sets.value().push(this.cite);
	//print("CloseCite");
	}
};

/**
 * Move variables tracking array into the array of
 * composed sets.
 */
CSL.Parallel.prototype.ComposeSet = function (next_output_in_progress) {
	var cite, pos, master, len;
	if (this.use_parallels) {
		// a bit loose here: zero-length sets relate to one cite,
		// apparently.
		if (this.sets.value().length === 1) {
			if (!this.in_series) {
				this.sets.value().pop();
				this.delim_counter += 1;
			}
			// XXXXX: hackaround that could be used maybe, if nothing cleaner pans out.
			//
			//print(this.sets.mystack.slice(-2,-1)[0].slice(-1)[0].back_forceme);
			//**print(this.sets.mystack.slice(-2,-1)[0].slice(-1)[0].back_forceme);
			//this.sets.mystack.slice(-2,-1)[0].slice(-1)[0].back_forceme = [];
		} else {
			len = this.sets.value().length;
			for (pos = 0; pos < len; pos += 1) {
				cite = this.sets.value()[pos];
				if (pos === 0) {
					this.delim_counter += 1;
				} else {
					if (!cite.Item.title && cite.use_journal_info) {
						this.delim_pointers.push(false);
					} else {
						this.delim_pointers.push(this.delim_counter);
					}
					this.delim_counter += 1;
				}

				if (CSL.POSITION_FIRST === cite.position) {
					if (pos === 0) {
						this.state.registry.registry[cite.itemId].master = true;
						//this.state.registry.registry[cite.itemId].parallel = cite.itemId;
						this.state.registry.registry[cite.itemId].siblings = [];
					} else {
						if (cite.prevItemID) {
							if (!this.state.registry.registry[cite.prevItemID].parallel) {
								this.state.registry.registry[cite.itemId].parallel = cite.prevItemID;
							} else {
								this.state.registry.registry[cite.itemId].parallel = this.state.registry.registry[cite.prevItemID].parallel;
							}
							this.state.registry.registry[cite.itemId].siblings = this.state.registry.registry[cite.prevItemID].siblings;
							this.state.registry.registry[cite.itemId].siblings.push(cite.itemId);
						}
					}
				}
			}
			this.sets.push([]);
			//this.in_series = false;

		}
		this.in_series = true;
		//print(this.sets.mystack.slice(-2,-1)[0].slice(-1)[0].back_forceme);
	}
};

/**
 * Mangle the queue as appropropriate.
 */
CSL.Parallel.prototype.PruneOutputQueue = function () {
	var len, pos, series, ppos, llen, cite;
	if (this.use_parallels) {
		len = this.sets.mystack.length;
		for (pos = 0; pos < len; pos += 1) {
			series = this.sets.mystack[pos];
			if (series.length > 1) {
				llen = series.length;
				for (ppos = 0; ppos < llen; ppos += 1) {
					cite = series[ppos];
					if (ppos === 0) {
						this.purgeVariableBlobs(cite, cite.back);
					} else if (ppos === (series.length - 1)) {
						//print(" a little rumor: "+cite.back_forceme);
						this.purgeVariableBlobs(cite, cite.front.concat(cite.back_forceme));
					} else {
						this.purgeVariableBlobs(cite, cite.front.concat(cite.back));
					}

				}
			}
		}
	}
};

CSL.Parallel.prototype.purgeVariableBlobs = function (cite, varnames) {
	var len, pos, varname, b, llen, ppos, out;
	if (this.use_parallels) {
		//
		// special delimiter within parallel cites.
		//
		out = this.state.output.current.value();
		if ("undefined" === typeof out.length) {
			out = out.blobs;
		}
		for (pos = 0, len = this.delim_pointers.length; pos < len; pos += 1) {
			ppos = this.delim_pointers[pos];
			if (ppos !== false) {
				out[ppos].parallel_delimiter = ", ";
			}
		}
		len = varnames.length - 1;
		for (pos = len; pos > -1; pos += -1) {
			varname = varnames[pos];
			if (cite[varname]) {
				llen = cite[varname].blobs.length - 1;
				for (ppos = llen; ppos > -1; ppos += -1) {
					b = cite[varname].blobs[ppos];
					b[0].blobs = b[0].blobs.slice(0, b[1]).concat(b[0].blobs.slice((b[1] + 1)));
				}
			}
		}
	}
};

