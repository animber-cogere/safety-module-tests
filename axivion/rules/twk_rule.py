# Axivion Bauhaus Suite
# http://www.axivion.com/
# Copyright (C) 2049-2020 Axivion GmbH

import re
import typing
from bauhaus import analysis
from bauhaus.ir.common.output import unparse


class TWKRule(analysis.ConfigurableRule):
    languages = ["C", "C++"]

    def __init__(self, rulename: str, *, rulegroup: str = None, **kwargs):
        rulegroup = "TWK"
        super().__init__(rulename, rulegroup=rulegroup, **kwargs)

    def get_rule_html_body():
        return None

    def get_rule_metadata(self):
        return ""

    def get_rulehtml_description(self, config=None):
        html = self.get_rule_metadata()
        html_body = self.get_rule_html_body()
        if html_body:
            html += "<h3>Description</h3>"
            html += html_body
        return html


@analysis.rule("VariableNaming")
class VariableNaming(TWKRule, analysis.AnalysisRule):
    title = "VariableNaming"
    severity = "required"

    const_pattern = "^[A-Z][A-Z0-9_]*$"
    """Pattern for const variables."""

    nonconst_pattern = "^([a-z][a-z0-9]*)([A-Z][a-z0-9]*)*$"
    """Pattern for non-const variables."""

    _message_descriptions = {
        "const_naming": "const variable name does not match the pattern '{}'.",
        "nonconst_naming": "Variable name does not match the pattern '{}'."
    }

    def execute(self, ir_graph):
        const_re = re.compile(self.const_pattern)
        nonconst_re = re.compile(self.nonconst_pattern)
        for node in self.relevant_pir_nodes(ir_graph, ["Global_Variable_Definition", "Local_Variable_Definition"]):
            name = node.Name
            type_ = node.Its_Type.skip_typedefs()
            if type_.is_of_type("Flags_Qualified_Type_Definition") and type_.Qualifiers.is_const:
                the_re = const_re
                key = "const_naming"
                pattern = self.const_pattern
            else:
                the_re = nonconst_re
                key = "nonconst_naming"
                pattern = self.nonconst_pattern
            if not the_re.match(name):
                self.add_message(node, key, entity=unparse.entity(node), message_arguments=(pattern,))

    def get_rule_html_body(self):
        return """<p>Variable naming convention.</p>"""

@analysis.rule("TypeClassifiers")
class TypeClassifierRule(TWKRule, analysis.AnalysisRule):
    title = "TypeClassifiers"
    severity = "required"

    _message_descriptions = {
        "qualifier_position": "The const and volatile qualifiers must be placed on the type's right-hand side.",
        "test": "Test"
    }

    def execute(self, ir_graph):
        for node in self.relevant_pir_nodes(
            ir_graph,
            [
                "Global_Variable_Definition",
                "Local_Variable_Definition",
                "Parameter_Definition",
            ],
        ):
            left_pos = node.Specifier_Position
            token = node.Token
            stop = False
            while token.Column != left_pos.Column or token.Line != left_pos.Line:
                try:
                    token = token.prev()
                except:
                    stop = True
                    break
            if stop:
                continue
            if token.Value in ("const", "volatile"):
                self.add_message(
                    token, "qualifier_position", entity=unparse.entity(token)
                )

    def get_rule_html_body(self):
        return """
<p>Type classifiers should be noted on the right-hand side of types.</p>
<p>
    <b>Example:</b>
    <pre><code>
int const x = 1; // ok
const int x = 1; // not ok
    </code></pre>
</p>
"""


@analysis.rule('TWK', parent_rulegroup='Stylechecks')
class IntentaGroup(analysis.rule_group.ConfigurableRuleGroup):
    @property
    def title(self) -> str:
        return 'TWK Rules.'

    def configuration(
        self, include_description=True
    ) -> typing.Mapping[str, analysis.ConfigOption]:
        from bauhaus.analysis.analysis_rule import DummyAnalysisRule

        return {
            **DummyAnalysisRule(
                'dummy',
                ctx=self._instantiation_context,
            ).configuration(include_description),
            **super().configuration(include_description),
        }
