from typing import IO
from pydantic import BaseModel
from reportlab.lib import colors
from reportlab.lib.pagesizes import LETTER, inch  # type: ignore
from reportlab.lib.styles import StyleSheet1, getSampleStyleSheet
from reportlab.platypus import PageBreak, Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle

from ...parameter import BaseParameterGroupModel, ParameterModel, ParameterGroupModel
from ..abstract_generator import AbstractGenerator, ModelContext

TABLESTYLE = TableStyle(
    [
        ("BACKGROUND", (0, 0), (-1, 0), colors.grey),
        ("TEXTCOLOR", (0, 0), (-1, 0), colors.whitesmoke),
        ("ALIGN", (0, 0), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("GRID", (0, 0), (-1, -1), 1, colors.black),
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("FONTSIZE", (0, 0), (-1, 0), 12),
        ("FONTSIZE", (0, 1), (-1, -1), 10),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
        ("TOPPADDING", (0, 0), (-1, 0), 8),
        ("BOTTOMPADDING", (0, 1), (-1, -1), 6),
        ("TOPPADDING", (0, 1), (-1, -1), 6),
    ]
)


def generate_group_description(group: BaseParameterGroupModel, doc: SimpleDocTemplate, styles: StyleSheet1) -> list:
    content = []
    # Add top level group
    # content.append(Paragraph(group._name.title(), styles["Heading1"]))

    group_title = group._name.title()

    section_title = Paragraph(f'<a name="{group_title}"/>{group_title}', styles["Heading1"])
    section_title.bookmark = group_title  # type: ignore
    content.append(section_title)

    # Add description
    content.append(Paragraph(group.resolved_description, styles["Normal"]))

    if group._parent and isinstance(group._parent, BaseParameterGroupModel):
        toc_html = f'<a href="#{group._parent._name.title()}">[Go to parent]</a>'
        content.append(Paragraph(toc_html, styles["Normal"]))

    content.append(Spacer(1, 0.25 * inch))
    # Add a table containing all the nodes
    table_data: list[list[str] | list[Paragraph]] = [
        ["Name", "Type", "ID", "Description"],
    ]
    for name, child in group.children.items():
        if isinstance(child, ParameterModel):
            table_data.append(
                [
                    Paragraph(name, styles["Normal"]),
                    Paragraph(child.primitive, styles["Normal"]),
                    Paragraph(f"0x{child.global_id:08X}", styles["Normal"]),
                    Paragraph(child.resolved_description, styles["Normal"]),
                ]
            )
        elif isinstance(child, ParameterGroupModel):
            table_data.append(
                [
                    Paragraph(f'<a href="#{child._name.title()}">{name}</a>', styles["Normal"]),
                    Paragraph("group", styles["Normal"]),
                    Paragraph(f"0x{child.global_id:08X}", styles["Normal"]),
                    Paragraph(child.resolved_description, styles["Normal"]),
                ]
            )

    table = Table(table_data, colWidths=[1.5 * inch, 0.8 * inch, 1 * inch, 3 * inch])
    table.setStyle(TABLESTYLE)

    content.append(table)
    content.append(Spacer(1, 0.5 * inch))

    # page break
    content.append(PageBreak())
    return content


class DocGenerator(AbstractGenerator):
    class Config(BaseModel):
        pass

    config: BaseModel

    def __init__(self, config: BaseModel):
        super().__init__(config)
        self.config = config

    def generate(self, model_context: ModelContext, output_path: str | IO) -> None:
        # Create PDF document
        doc = SimpleDocTemplate(output_path, pagesize=LETTER, rightMargin=40, leftMargin=40, topMargin=40, bottomMargin=40)

        # Stylesheet for consistent styling
        styles = getSampleStyleSheet()

        # Elements list
        content = []

        # Title aligned center
        content.append(Paragraph("ODIN Object Dictionary", styles["Title"]))
        content.append(Spacer(1, 0.25 * inch))
        models = [model_context.root_model]
        models.extend(model_context.root_model.to_flat_list())  # type: ignore

        for parmeter in models:
            if not isinstance(parmeter, BaseParameterGroupModel):
                continue

            content += generate_group_description(parmeter, doc, styles)

        # Generate PDF
        doc.build(content)
